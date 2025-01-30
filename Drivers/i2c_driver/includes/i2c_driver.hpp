#include <stdint.h>

class I2cBus
{
    private:
        uint32_t a;
        uint32_t b;

    public:
        I2cBus(uint32_t a, uint32_t b);
};