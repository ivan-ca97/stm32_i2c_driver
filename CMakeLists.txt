cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME stm32_i2c_driver)

# Enable CMake support for ASM and C languages
enable_language(C ASM CXX)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Enable exceptions
target_compile_options(${PROJECT_NAME} PUBLIC -fexceptions)

target_link_options(${PROJECT_NAME} PRIVATE -Wl,--target2=rel)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--target2=rel")

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Core/Src/main_loop.cpp
    Drivers/i2c_driver/i2c_driver_exceptions.cpp
    Drivers/i2c_driver/i2c_interrupt_handlers.cpp
    Drivers/i2c_driver/i2c_transaction.cpp
    Drivers/i2c_driver/i2c_device.cpp
    Drivers/i2c_driver/i2c_bus.cpp
    Drivers/custom_exception/custom_exception.cpp
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
# Add user defined include paths
    Drivers/i2c_driver/includes
    Drivers/custom_exception/includes
    Drivers/queue/includes
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
