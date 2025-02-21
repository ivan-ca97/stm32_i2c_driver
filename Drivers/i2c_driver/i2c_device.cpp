#include "i2c_device.hpp"


I2cDevice::I2cDevice(uint16_t address, I2cBus* bus, std::string name)
    : address(address), bus(bus), name(name)
{

}

uint16_t I2cDevice::getAddress(void)
{
    return address;
}

void I2cDevice::attachBus(I2cBus* bus)
{
    if(this->bus != nullptr)
    {
        throw I2cException("Device already attached to a bus");
    }

    this->bus = bus;
}

void I2cDevice::detachBus()
{
    this->bus = nullptr;
}

void I2cDevice::setTransaction(I2cTransaction &transaction)
{
    bus->setTransaction(transaction);
}