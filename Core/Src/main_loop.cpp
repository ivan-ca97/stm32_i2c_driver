#include <stdint.h>
#include "main.h"

#include "i2c_bus.hpp"
#include "i2c_device.hpp"

#include "queue.hpp"

#define I2C_BUFFER_SIZE 10

void postReadCallback(void* params)
{
    bool *transactionInProgress = reinterpret_cast<bool*>(params);
    *transactionInProgress = false;
}

void errorReadCallback(void* params)
{
    bool *transactionError = reinterpret_cast<bool*>(params);
    *transactionError = true;
}

void postConfigCallback(void* params)
{
    bool *configReady = reinterpret_cast<bool*>(params);
    *configReady = true;
}

void initGpio()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    // Configurar PB5 como entrada
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Habilitar y configurar la interrupción EXTI5
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0); // Prioridad para EXTI5 (líneas 5-9)
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

// Handler de la interrupción EXTI (definido en stm32f4xx_it.c)
extern "C" void EXTI9_5_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5); // Llamar al handler de HAL para PB5
}

bool conversionReady = false;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == GPIO_PIN_5) {
    conversionReady = true; // Marcar que ALERT/RDY se activó
  }
}

void i2cRecover();

bool loop(void)
{
    bool rdyPin;
    initGpio();

    i2cRecover();

    try
    {
        bool configReady = false;
        uint8_t txConfig[2], txThreshLow[2], txThresHigh[2];

        StaticQueue<I2cTransaction, I2C_BUFFER_SIZE> i2cBuffer;

        I2cBus i2cBus("Bus number 1", &i2cBuffer, I2C_BUS_1, 10000);
        //I2cDevice i2cAdc(0x48, &i2cBus, "ADC_1");
        I2cDevice i2cAdc(0x4A, &i2cBus, "ADC_1");

        //I2C_HandleTypeDef* handleI2c = i2cBus.getHandle();

        txConfig[0] = 0xC4;  // MSB (0xC480 -> MSB = 0xC4)
        txConfig[1] = 0x80;  // LSB (0xC480 -> LSB = 0x80)

        txThreshLow[0] = 0x00;
        txThreshLow[1] = 0x00;

        txThresHigh[0] = 0x80;
        txThresHigh[1] = 0x00;

        I2cTransaction configAdc(TRANSACTION_TX, txConfig, 2, &i2cAdc, 0x01, REGISTER_8_BITS);
        I2cTransaction configAdcThreshLow(TRANSACTION_TX, txThreshLow, 2, &i2cAdc, 0x02, REGISTER_8_BITS);
        I2cTransaction configAdcThreshHigh(TRANSACTION_TX, txThresHigh, 2, &i2cAdc, 0x03, REGISTER_8_BITS);

        configAdcThreshHigh.setPostCallback(postConfigCallback, &configReady);

        configAdc.send();
        configAdcThreshLow.send();
        configAdcThreshHigh.send();

        while(!configReady);

        bool transactionInProgress = true;
        bool transactionError = false;
        bool conversionSkip;
        uint16_t data = 0;
        uint8_t rxBuffer[2] = {0, 0};
        while(true)
        {
            // Wait until conversion is done
            conversionSkip = false;
            while(!conversionReady)
            {
                if(conversionSkip)
                    break;
                rdyPin = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET;
            }
            conversionReady = false;

            I2cTransaction transactionRead = I2cTransaction::I2cRxTransaction(&i2cAdc, rxBuffer, 2, 0x00, REGISTER_8_BITS);

            rdyPin = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET;
            transactionRead.setPostCallback(postReadCallback, &transactionInProgress);
            transactionRead.setErrorCallback(errorReadCallback, &transactionError);

            transactionInProgress = true;
            transactionRead.send();

            // Wait until transaction finishes.
            while(transactionInProgress)
            {
                if(transactionError)
                {
                    i2cRecover();
                    transactionError = false;
                    break;
                }
            }

            data = (rxBuffer[0] << 8) | rxBuffer[1];
        }
    }
    catch(I2cException& e)
    {
        while(true);
    }
}

void i2cRecover()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    int SDA_PIN = GPIO_PIN_7;
    int SCL_PIN = GPIO_PIN_6;

    // Configura SCL y SDA como GPIO open-drain
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7; // Ej: PB6 para I2C1 en STM32F4
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Genera 9 pulsos en SCL para liberar el esclavo
    for (int i = 0; i < 9; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOB, SCL_PIN, GPIO_PIN_SET);
        HAL_Delay(1);
    }

    // Genera un STOP manual
    HAL_GPIO_WritePin(GPIOB, SDA_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, SCL_PIN, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, SDA_PIN, GPIO_PIN_SET);
    HAL_Delay(1);
}