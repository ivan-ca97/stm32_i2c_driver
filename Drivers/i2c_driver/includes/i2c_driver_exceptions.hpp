#pragma once

#include "custom_exception.hpp"

class I2cException : public CustomException {
    public:
        explicit I2cException(const std::string& message);

        explicit I2cException(void);
};