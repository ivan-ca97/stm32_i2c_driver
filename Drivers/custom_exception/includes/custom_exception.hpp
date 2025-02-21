#pragma once

#include <exception>
#include <string>

class CustomException : public std::exception
{
    protected:
        std::string message = "An exception has occurred";
    public:
        explicit CustomException(const std::string& msg);

        const char* what() const noexcept override;
};