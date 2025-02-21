#pragma once

#include <stdint.h>

class I2cDevice;

typedef void (*Callback)(void*);

typedef enum
{
    TRANSACTION_RX,
    TRANSACTION_TX
}
TransactionDirection;

typedef enum
{
    REGISTER_NULL,
    REGISTER_8_BITS,
    REGISTER_16_BITS
}
RegisterLength;
 
class I2cTransaction
{
    protected:
        TransactionDirection direction;
        uint16_t address;
        uint8_t* data;
        uint16_t dataBytes;
        I2cDevice* device;
        uint16_t deviceRegister;
        RegisterLength deviceRegisterBytes;

        void* preCallbackParameters = nullptr;
        void* postCallbackParameters = nullptr;
        Callback preCallbackFunction = nullptr;
        Callback postCallbackFunction = nullptr;

    public:
        I2cTransaction();

        static I2cTransaction I2cTxTransaction(I2cDevice *device, uint8_t* data, uint16_t dataBytes, uint16_t deviceRegister = 0, RegisterLength deviceRegisterBytes = REGISTER_NULL);

        static I2cTransaction I2cRxTransaction(I2cDevice *device, uint8_t* data, uint16_t dataBytes, uint16_t deviceRegister = 0, RegisterLength deviceRegisterBytes = REGISTER_NULL);
    
        I2cTransaction(TransactionDirection direction, uint8_t* data, uint16_t dataBytes, I2cDevice *device, uint16_t address, uint16_t deviceRegister = 0, RegisterLength deviceRegisterBytes = REGISTER_NULL);

        I2cTransaction(TransactionDirection direction, uint8_t* data, uint16_t dataBytes, uint16_t address, uint16_t deviceRegister = 0, RegisterLength deviceRegisterBytes = REGISTER_NULL);
    
        I2cTransaction(TransactionDirection direction, uint8_t* data, uint16_t dataBytes, I2cDevice *device, uint16_t deviceRegister = 0, RegisterLength deviceRegisterBytes = REGISTER_NULL);

        void setPreCallback(Callback callback, void* parameters);

        void setPostCallback(Callback callback, void* parameters);

        uint16_t getAddress(void);

        uint8_t* getDataPointer(void);

        uint16_t getDataLenthBytes(void);
        
        uint16_t getRegister(void);
        
        RegisterLength getRegisterBytes(void);

        TransactionDirection getDirection();

        /*
         *  @brief Calls the pre-transaction callback before the transaction is set with the configured parameters.
         */
        void preCallback(void);
    
        /*
         *  @brief Calls the post-transaction callback after the transaction is finished with the configured parameters.
         */
        void postCallback(void);

        void send(void);
};