#include "i2c_bus.hpp"

#include <algorithm>

#include "stm32f4xx_it.h"

#define I2C_FAST_MODE_CUTOFF_FREQUENCY 100000


// Initialize with empty drivers array.
std::array<I2cBus*, I2C_BUS_MAX> I2cBus::drivers = {};

I2C_HandleTypeDef* I2cBus::getHandle(void)
{
    return &handle;
}

void I2cBus::transactionCompleteCallback(I2C_HandleTypeDef *handle)
{
    // Get the bus object
    I2cBus* bus = reinterpret_cast<I2cBus*>(
        reinterpret_cast<uint8_t*>(handle) - offsetof(I2cBus, handle)
    );

    // Dequeues and calls the post-transaction callback once it's complete.
    I2cTransaction transaction = bus->queue->dequeue();
    transaction.postCallback();    

    bus->sendNextTransaction();
}

void I2cBus::sendNextTransaction(void)
{
    currentTransaction = queue->peek();
    if(!currentTransaction)
    {
        return;
    }

    currentTransaction->preCallback();

    sendTransaction(*currentTransaction);
}

void I2cBus::sendTransaction(I2cTransaction &transaction)
{
    HAL_StatusTypeDef error;
    TransactionDirection direction = transaction.getDirection();

    uint16_t address = transaction.getAddress();
    uint8_t* data = transaction.getDataPointer();
    uint16_t dataSize = transaction.getDataLenthBytes();
    uint8_t deviceRegister = transaction.getRegister();
    RegisterLength deviceRegisterBytes = transaction.getRegisterBytes();

    if(deviceRegisterBytes != REGISTER_NULL)
    {
        uint8_t deviceRegisterSize = (deviceRegisterBytes == REGISTER_8_BITS ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT);
        switch(direction)
        {
            case TRANSACTION_RX:
                error = HAL_I2C_Mem_Read_IT(&handle, address << 1, deviceRegister, deviceRegisterSize, data, dataSize);
                break;
            case TRANSACTION_TX:
                error = HAL_I2C_Mem_Write_IT(&handle, address << 1, deviceRegister, deviceRegisterSize, data, dataSize);
                break;
        }
    }
    else
    {
        switch(direction)
        {
            case TRANSACTION_RX:
                error = HAL_I2C_Master_Receive_IT(&handle, address << 1, data, dataSize);
                break;
            case TRANSACTION_TX:
                error = HAL_I2C_Master_Transmit_IT(&handle, address << 1, data, dataSize);
                break;
        }
    }

    if(error != HAL_OK)
    {
        throw I2cException("There was an error setting up the transaction.");
    }
}

void I2cBus::setTransaction(I2cTransaction &transaction)
{
    queue->enqueue(transaction);

    if(queue->size() == 1 && HAL_I2C_GetState(&handle) == HAL_I2C_STATE_READY)
    {
        sendNextTransaction();
    }
}

void I2cBus::handleInterrupt(I2cBusSelection bus, I2cInterruptType type)
{
    I2cBus *driver = I2cBus::drivers[bus];
    if(driver)
    {
        switch(type)
        {
        case I2C_EVENT:
            HAL_I2C_EV_IRQHandler(&driver->handle);
            break;
        case I2C_ERROR:
            HAL_I2C_ER_IRQHandler(&driver->handle);
            break;
        }
    }
}

I2cBus::I2cBus(
    std::string name,
    Queue<I2cTransaction> *queue,
    I2cBusSelection bus,
    uint32_t clockSpeed,
    bool addressing7Bit,
    I2cDutyCycle dutyCycle,
    bool masterOnly,
    bool dualAddress,
    uint16_t ownAddress1,
    uint16_t ownAddress2,
    bool clockStretching,
    bool generalCall
) : name(name), queue(queue), bus(bus)
{
    registerDriver(bus);

    if(clockSpeed <= I2C_FAST_MODE_CUTOFF_FREQUENCY)
    {
        this->fastMode = false;
    }

    if(masterOnly)
    {
        clockStretching = false;
        generalCall = false;
        dualAddress = false;
        ownAddress1 = 0x0;
        ownAddress2 = 0x0;
    }

    if(!masterOnly)
    {
        areAddressesValid(ownAddress1, ownAddress2, dualAddress, addressing7Bit);
    }

    initHandle(clockSpeed, addressing7Bit, dutyCycle, dualAddress, generalCall, clockStretching, ownAddress1, ownAddress2);

    initGpio();
    initNvic();
}

bool I2cBus::areAddressesValid(uint16_t ownAddress1, uint16_t ownAddress2, bool dualAddress, bool addressing7bit)
{
    if(dualAddress)
    {
        if(checkAddressValidity(ownAddress1, addressing7bit) && checkAddressValidity(ownAddress2, addressing7bit))
        {
            return true;
        }
    }
    else
    {
        if(checkAddressValidity(ownAddress1, addressing7bit))
        {
            return true;
        }
    }

    throw I2cException("The provided I2C addresses are not valid");
}

bool I2cBus::checkAddressValidity(uint16_t address, bool addressing7bit)
{
    // Addresses under 0x0F and over 0x78 are reserved in the I2C standard.
    if(addressing7bit && (address <= 0x0F || address >= 0x78))
    {
        return false;
    }

    // Check that the address exceeds the 10 bit range.
    if(!addressing7bit && address > 0x3FF)
    {
        return false;
    }

    return true;
}

I2cBusSelection I2cBus::getBusNumber(void)
{
    return bus;
}

void I2cBus::registerDriver(I2cBusSelection bus)
{
    uint8_t i;

    switch(bus)
    {
        case I2C_BUS_1:
            i = 0;
            break;
        case I2C_BUS_2:
            i = 1;
            break;
        case I2C_BUS_3:
            i = 2;
            break;
        default:
            throw I2cException();
    }
    
    if(drivers[i] != nullptr)
        throw I2cException("Bus already in use");

    drivers[i] = this;
}

void I2cBus::initHandle(
    uint32_t clockSpeed,
    bool addressing7Bit,
    I2cDutyCycle dutyCycle,
    bool generalCall,
    bool clockStretching,
    bool dualAddress,
    uint16_t ownAddress1,
    uint16_t ownAddress2
)
{
    I2C_TypeDef *i2cInstance;
    switch(bus)
    {
        case I2C_BUS_1:
            i2cInstance = I2C1;
            break;
        case I2C_BUS_2:
            i2cInstance = I2C2;
            break;
        case I2C_BUS_3:
            i2cInstance = I2C3;
            break;
        default:
            throw I2cException();
    }

    handle.Instance = i2cInstance;
    handle.Init.ClockSpeed = clockSpeed;
    handle.Init.AddressingMode = addressing7Bit ? I2C_ADDRESSINGMODE_7BIT : I2C_ADDRESSINGMODE_10BIT;
    // Duty cycle configuration is only taken into account when fast mode is used.
    handle.Init.DutyCycle = dutyCycle;

    // Slave configurations
    handle.Init.GeneralCallMode = generalCall ? I2C_GENERALCALL_ENABLE : I2C_GENERALCALL_DISABLE;
    handle.Init.NoStretchMode = clockStretching ? I2C_NOSTRETCH_ENABLE : I2C_NOSTRETCH_DISABLE;

    handle.Init.DualAddressMode = dualAddress ? I2C_DUALADDRESS_ENABLE : I2C_DUALADDRESS_DISABLE;
    handle.Init.OwnAddress1 = ownAddress1;
    handle.Init.OwnAddress2 = ownAddress2;

    handle.MspInitCallback = nullptr;
    
    if(HAL_I2C_Init(&handle) != HAL_OK)
    {
        throw I2cException("There was an error initializing the I2C driver.");
    }

    registerCallbacks();
}


void I2cBus::registerCallbacks(void)
{
    if(HAL_I2C_RegisterCallback(&handle, HAL_I2C_MASTER_TX_COMPLETE_CB_ID, transactionCompleteCallback) != HAL_OK)
    {
        throw I2cException("There was an error registering the callback.");
    }

    if(HAL_I2C_RegisterCallback(&handle, HAL_I2C_MASTER_RX_COMPLETE_CB_ID, transactionCompleteCallback) != HAL_OK)
    {
        throw I2cException("There was an error registering the callback.");
    }

    if(HAL_I2C_RegisterCallback(&handle, HAL_I2C_MEM_TX_COMPLETE_CB_ID, transactionCompleteCallback) != HAL_OK)
    {
        throw I2cException("There was an error registering the callback.");
    }

    if(HAL_I2C_RegisterCallback(&handle, HAL_I2C_MEM_RX_COMPLETE_CB_ID, transactionCompleteCallback) != HAL_OK)
    {
        throw I2cException("There was an error registering the callback.");
    }

    if(HAL_I2C_RegisterCallback(&handle, HAL_I2C_MASTER_RX_COMPLETE_CB_ID, transactionCompleteCallback) != HAL_OK)
    {
        throw I2cException("There was an error registering the callback.");
    }

    // if(HAL_I2C_RegisterCallback(&handle, HAL_I2C_ERROR_CB_ID, transactionCompleteCallback) != HAL_OK)
    // {
    //     throw I2cException("There was an error registering the callback.");
    // }

    if(HAL_I2C_RegisterCallback(&handle, HAL_I2C_ABORT_CB_ID, transactionCompleteCallback) != HAL_OK)
    {
        throw I2cException("There was an error registering the callback.");
    }
}

void I2cBus::initGpio(void)
{
    uint32_t pinMaskA = 0;
    uint32_t pinMaskB = 0;

    uint32_t alternateFunction = 0;

    switch(bus)
    {
        case I2C_BUS_1:
            __HAL_RCC_I2C1_CLK_ENABLE();

            pinMaskB = GPIO_PIN_6 | GPIO_PIN_7;
            alternateFunction = GPIO_AF4_I2C1;
            break;
        case I2C_BUS_2:
            __HAL_RCC_I2C2_CLK_ENABLE();

            pinMaskB = GPIO_PIN_3 | GPIO_PIN_10;
            alternateFunction = GPIO_AF4_I2C2;
            break;
        case I2C_BUS_3:
            __HAL_RCC_I2C3_CLK_ENABLE();

            pinMaskA = GPIO_PIN_8;
            pinMaskB = GPIO_PIN_4;
            alternateFunction = GPIO_AF4_I2C3;
            break;
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {
        .Pin = pinMaskB,
        .Mode = GPIO_MODE_AF_OD,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = alternateFunction
    };

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    if(pinMaskA)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin = pinMaskA;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

void I2cBus::initNvic(void)
{
    IRQn_Type eventInterrupt;
    IRQn_Type errorInterrupt;

    switch(bus)
    {
        case I2C_BUS_1:
            eventInterrupt = I2C1_EV_IRQn;
            errorInterrupt = I2C1_ER_IRQn;
            break;
        case I2C_BUS_2:
            eventInterrupt = I2C2_EV_IRQn;
            errorInterrupt = I2C2_ER_IRQn;
            break;
        case I2C_BUS_3:
            eventInterrupt = I2C3_EV_IRQn;
            errorInterrupt = I2C3_ER_IRQn;
            break;
    }

    HAL_NVIC_SetPriority(eventInterrupt, 1, 1);
    HAL_NVIC_EnableIRQ(eventInterrupt);
    HAL_NVIC_SetPriority(errorInterrupt, 1, 1);
    HAL_NVIC_EnableIRQ(errorInterrupt);
}