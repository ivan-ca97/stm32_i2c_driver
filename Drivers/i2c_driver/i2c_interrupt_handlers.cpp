#include "i2c_bus.hpp"

/*
 *  Interrupt handlers by driver
 */
extern "C" void I2C1_EV_IRQHandler(void)
{
    I2cBus::handleInterrupt(I2C_BUS_1, I2C_EVENT);
}

extern "C" void I2C2_EV_IRQHandler(void)
{
    I2cBus::handleInterrupt(I2C_BUS_2, I2C_EVENT);
}

extern "C" void I2C3_EV_IRQHandler(void)
{
    I2cBus::handleInterrupt(I2C_BUS_3, I2C_EVENT);
}

extern "C" void I2C1_ER_IRQHandler(void)
{
    I2cBus::handleInterrupt(I2C_BUS_1, I2C_ERROR);
}

extern "C" void I2C2_ER_IRQHandler(void)
{
    I2cBus::handleInterrupt(I2C_BUS_2, I2C_ERROR);
}

extern "C" void I2C3_ER_IRQHandler(void)
{
    I2cBus::handleInterrupt(I2C_BUS_3, I2C_ERROR);
}