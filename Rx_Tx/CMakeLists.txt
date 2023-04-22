# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

#set(CMAKE_C_STANDARD 11)
#set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "C:/Program Files/Raspberry Pi/Pico SDK v1.5.0/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Rx_Tx)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# LIbrerias externas
#add_subdirectory(libs)

# Add executable. Default name is the project name, version 0.1

add_executable(Rx_Tx 
    src/main.c
    src/RX_TX.c
)

pico_set_program_name(Rx_Tx "Rx_Tx")
pico_set_program_version(Rx_Tx "0.1")

pico_enable_stdio_uart(Rx_Tx 0)
pico_enable_stdio_usb(Rx_Tx 1)

target_include_directories(Rx_Tx PUBLIC inc/)

# Add the standard library to the build
target_link_libraries(Rx_Tx pico_stdlib 
hardware_spi
hardware_gpio) #Agrego el nombre de la libreria a usar

pico_add_extra_outputs(Rx_Tx)

