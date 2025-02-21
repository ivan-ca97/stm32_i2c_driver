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

bool loop(void)
{
    try
    {
        uint8_t txBuffer[2];

        StaticQueue<I2cTransaction, I2C_BUFFER_SIZE> i2cBuffer;

        I2cBus i2cBus("Bus number 1", &i2cBuffer, I2C_BUS_1, 10000);
        I2cDevice i2cAdc(0x48, &i2cBus, "ADC_1");

        //I2C_HandleTypeDef* handleI2c = i2cBus.getHandle();

        txBuffer[0] = 0xC4;  // MSB (0xC483 -> MSB = 0xC4)
        txBuffer[1] = 0x83;  // LSB (0xC483 -> LSB = 0x83)
        I2cTransaction configAdc(TRANSACTION_TX, txBuffer, 2, &i2cAdc, 0x01, REGISTER_8_BITS);
        configAdc.send();


        bool transactionInProgress = true;
        uint16_t data = 0;
        uint8_t rxBuffer[2] = {0, 0};
        while(true)
        {
            I2cTransaction transactionRead2 = I2cTransaction::I2cRxTransaction(&i2cAdc, rxBuffer, 2, 0x00, REGISTER_8_BITS);

            transactionRead2.setPostCallback(postReadCallback, &transactionInProgress);

            transactionInProgress = true;
            transactionRead2.send();

            // Wait until transaction finishes.
            while(transactionInProgress);

            data = (rxBuffer[0] << 8) | rxBuffer[1];
        }
    }
    catch(I2cException& e)
    {
        while(true);
    }
}