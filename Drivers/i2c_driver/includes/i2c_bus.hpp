#pragma once

#include <stdint.h>
#include <array>
#include "stm32f4xx_hal.h"

#include "i2c_driver_exceptions.hpp"
#include "i2c_transaction.hpp"

#include "queue.hpp"

#define I2C_BUS_MAX 3

typedef enum
{
    I2C_BUS_1,
    I2C_BUS_2,
    I2C_BUS_3
}
I2cBusSelection;

typedef enum
{
    I2C_EVENT,
    I2C_ERROR
}
I2cInterruptType;

typedef enum
{
    I2C_DUTY_CYCLE_2,
    I2C_DUTY_CYCLE_16_9

}
I2cDutyCycle;

#ifdef __cplusplus
extern "C" {
#endif
void I2C1_ER_IRQHandler(void);
void I2C1_EV_IRQHandler(void);
void I2C2_ER_IRQHandler(void);
void I2C2_EV_IRQHandler(void);
void I2C3_ER_IRQHandler(void);
void I2C3_EV_IRQHandler(void);
#ifdef __cplusplus
}
#endif

class I2cDevice;

class I2cBus
{
    protected:
        static std::array<I2cBus*, I2C_BUS_MAX> drivers;

        static void handleInterrupt(I2cBusSelection bus, I2cInterruptType type);

        Queue<I2cTransaction> *queue;

        I2cTransaction* currentTransaction;

        I2C_HandleTypeDef handle = {};

        I2cBusSelection bus;

        bool fastMode;

        std::string name;

        /*
         *  @brief Initializes the I2C handle with the given parameters
         *
         *	@param clockSpeed Clock speed to be configured for the bus.
         *	@param addressing7Bit Use 7 bit addresses if true. 10 bit addresses if false.
         *	@param dutyCycle Duty cycle type.
         *	@param generalCall Configure if the bus should accept general calls.
         *	@param clockStretching Configure if the bus should use clock stretching.
         *	@param dualAddress Configure if the bus should use dual addresses in slave mode.
         *	@param ownAddress1 Configure address 1 for the bus in slave mode.
         *	@param ownAddress2 Configure address 2 for the bus in slave mode.
         *
         *  @throws I2cException: If there's a HAL error.
         */
        void initHandle(
            uint32_t clockSpeed,
            bool addressing7Bit,
            I2cDutyCycle dutyCycle,
            bool generalCall,
            bool clockStretching,
            bool dualAddress,
            uint16_t ownAddress1,
            uint16_t ownAddress2
        );

        /*
         *  @brief Registers the interrupt callbacks to the HAL handle.
         *
         *  @throws I2cException: If there's a HAL error.
         */
        void registerCallbacks(void);

        void registerDriver(I2cBusSelection bus);

        void initGpio(void);

        void initNvic(void);

        /*
         *  @brief Checks whether the addresses are valid, taking into account the addressing mode
         *  (7 bit or 10 bit) and the dual or single address configuration
         *
         *	@param ownAddress1 Address 1 to be checked
         *	@param ownAddress2 Address 2 to be checked
         *	@param dualAddress Whether dual slave addresses mode is used.
         *	@param addressing7bit Addressing mode to consider (false: 10 bit- true: 7 bit)
         *
         *  @throws I2cException: When the provided parameters are not in a valid state.
         */
        bool areAddressesValid(uint16_t ownAddress1, uint16_t ownAddress2, bool dualAddress, bool addressing7bit);

        void sendTransaction(I2cTransaction &transaction);

        void sendNextTransaction(void);

        void setTransaction( I2cTransaction &transaction);

        static void transactionCompleteCallback(I2C_HandleTypeDef *handle);

    public:
        I2C_HandleTypeDef* getHandle(void);
        I2cBus(
            std::string name,
            Queue<I2cTransaction> *queue,
            I2cBusSelection bus,
            uint32_t clockSpeed,
            bool addressing7Bit = true,
            I2cDutyCycle dutyCycle = I2C_DUTY_CYCLE_2,
            bool masterOnly = true,
            bool dualAddress = false,
            uint16_t ownAddress1 = 0,
            uint16_t ownAddress2 = 0,
            bool clockStretching = false,
            bool generalCall = false
        );

        /*
         *  @brief Checks whether the address is valid, taking into account the addressing mode
         *  (7 bit or 10 bit)
         *
         *	@param address Address to be checked
         *	@param addressing7bit Addressing mode to consider (false: 10 bit- true: 7 bit)
         */
        bool checkAddressValidity(uint16_t address, bool addressing7bit);

        I2cBusSelection getBusNumber(void);

    friend class I2cDevice;

    // Interrupt handlers declared as friends
    friend void I2C1_EV_IRQHandler(void);

    friend void I2C2_EV_IRQHandler(void);

    friend void I2C3_EV_IRQHandler(void);

    friend void I2C1_ER_IRQHandler(void);

    friend void I2C2_ER_IRQHandler(void);

    friend void I2C3_ER_IRQHandler(void);
};