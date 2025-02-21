#include "custom_exception.hpp"


CustomException::CustomException(const std::string& msg) : message(msg)
{

}

const char* CustomException::what() const noexcept
{
    return message.c_str();
}