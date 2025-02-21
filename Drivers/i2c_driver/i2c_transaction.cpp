#include "i2c_transaction.hpp"
#include "i2c_device.hpp"



I2cTransaction::I2cTransaction()
    : data(nullptr), dataBytes(0), device(nullptr)
{

}

I2cTransaction I2cTransaction::I2cTxTransaction(I2cDevice *device, uint8_t* data, uint16_t dataBytes, uint16_t deviceRegister, RegisterLength deviceRegisterBytes)
{
    return I2cTransaction(TRANSACTION_TX, data, dataBytes, device, deviceRegister, deviceRegisterBytes);
}

I2cTransaction I2cTransaction::I2cRxTransaction(I2cDevice *device, uint8_t* data, uint16_t dataBytes, uint16_t deviceRegister, RegisterLength deviceRegisterBytes)
{
    return I2cTransaction(TRANSACTION_RX, data, dataBytes, device, deviceRegister, deviceRegisterBytes);
}

I2cTransaction::I2cTransaction(TransactionDirection direction, uint8_t* data, uint16_t dataBytes, I2cDevice *device, uint16_t address, uint16_t deviceRegister, RegisterLength deviceRegisterBytes)
    : direction(direction), address(address), data(data), dataBytes(dataBytes), device(device), deviceRegister(deviceRegister), deviceRegisterBytes(deviceRegisterBytes)
{
    if(deviceRegisterBytes == REGISTER_NULL && deviceRegister != 0)
        throw I2cException("Configured register but not register length");
}

I2cTransaction::I2cTransaction(TransactionDirection direction, uint8_t* data, uint16_t dataBytes, uint16_t address, uint16_t deviceRegister, RegisterLength deviceRegisterBytes)
    : I2cTransaction(direction, data, dataBytes, nullptr, address, deviceRegister, deviceRegisterBytes)
{

}

I2cTransaction::I2cTransaction(TransactionDirection direction, uint8_t* data, uint16_t dataBytes, I2cDevice *device, uint16_t deviceRegister, RegisterLength deviceRegisterBytes)
    : I2cTransaction(direction, data, dataBytes, device, device->getAddress(), deviceRegister, deviceRegisterBytes)
{

}

void I2cTransaction::setPreCallback(Callback callback, void* parameters)
{
    preCallbackFunction = callback;
    preCallbackParameters = parameters;
}

void I2cTransaction::setPostCallback(Callback callback, void* parameters)
{
    postCallbackFunction = callback;
    postCallbackParameters = parameters;
}

uint16_t I2cTransaction::getAddress(void)
{
    return address;
}

uint8_t* I2cTransaction::getDataPointer(void)
{
    return data;
}

uint16_t I2cTransaction::getDataLenthBytes(void)
{
    return dataBytes;
}

uint16_t I2cTransaction::getRegister(void)
{
    return deviceRegister;
}

RegisterLength I2cTransaction::getRegisterBytes(void)
{
    return deviceRegisterBytes;
}

TransactionDirection I2cTransaction::getDirection(void)
{
    return direction;
}

void I2cTransaction::send(void)
{
    if(!device)
        throw I2cException("Device for the I2cTransaction not set");

    device->setTransaction(*this);
}

void I2cTransaction::preCallback()
{
    if(preCallbackFunction)
        preCallbackFunction(preCallbackParameters);
}

void I2cTransaction::postCallback()
{
    if(postCallbackFunction)
        postCallbackFunction(postCallbackParameters);
}