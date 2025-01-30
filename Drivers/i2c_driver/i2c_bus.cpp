/*
 * i2c_bus.cpp
 *
 *  Created on: Jan 21, 2025
 *      Author: Ivan
 */

#include "i2c_driver.hpp"

int returnInt(void)
{
	return 1;
}

I2cBus::I2cBus(uint32_t a, uint32_t b)
{
	this->a = a;
	this->b = b;
}