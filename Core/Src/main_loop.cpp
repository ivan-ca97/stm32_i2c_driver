#include <stdint.h>
#include "main.h"

#include "i2c_driver.hpp"

bool loop(void)
{
    volatile uint32_t i = 0;

    I2cBus obj(111, 222);

    while(true)
    {
        i++;
    }
}