cmake_minimum_required(VERSION 3.13)

# Initialize pico-sdk
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(uniprog_bridge C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(uniprog_bridge
    src/main.c
    src/bridge_protocol.c
    src/executor_spi.c
    src/executor_i2c.c
    src/executor_sys.c
    src/executor_gpio.c
    src/usb_descriptors.c
)

# Pull in the existing HAL platform implementation
add_subdirectory(../src/platform/rp2040 rp2040_hal)
include_directories(
    include
    ../include
)

target_link_libraries(uniprog_bridge
    pico_stdlib
    tinyusb_device
    tinyusb_board
    rp2040_hal
)

pico_enable_stdio_usb(uniprog_bridge 0)
pico_enable_stdio_uart(uniprog_bridge 1)
pico_add_extra_outputs(uniprog_bridge)
