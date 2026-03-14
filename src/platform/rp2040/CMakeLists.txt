# Platform HAL Implementation for RP2040

add_library(rp2040_hal STATIC
    ${CMAKE_CURRENT_LIST_DIR}/rp2040_platform.c
    ${CMAKE_CURRENT_LIST_DIR}/rp2040_hal_gpio.c
    ${CMAKE_CURRENT_LIST_DIR}/rp2040_hal_spi.c
    ${CMAKE_CURRENT_LIST_DIR}/rp2040_hal_i2c.c
    ${CMAKE_CURRENT_LIST_DIR}/rp2040_hal_uart.c
    ${CMAKE_CURRENT_LIST_DIR}/rp2040_hal_timer.c
    ${CMAKE_CURRENT_LIST_DIR}/rp2040_hal_pio.c
)

target_include_directories(rp2040_hal PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/../../..
    ${CMAKE_CURRENT_LIST_DIR}/../../../include
    ${CMAKE_CURRENT_LIST_DIR}/../../../firmware/include
)

target_compile_definitions(rp2040_hal PUBLIC UNIPROGER_PLATFORM_RP2040)

# Generate PIO headers
pico_generate_pio_header(rp2040_hal ${CMAKE_CURRENT_LIST_DIR}/pio/jtag.pio)
pico_generate_pio_header(rp2040_hal ${CMAKE_CURRENT_LIST_DIR}/pio/swd.pio)

target_include_directories(rp2040_hal PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../../include
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(rp2040_hal PRIVATE
    pico_stdlib
    hardware_gpio
    hardware_spi
    hardware_i2c
    hardware_uart
    hardware_timer
    hardware_pio
    hardware_dma
    hardware_watchdog
    pico_unique_id
    tinyusb_device
    tinyusb_board
)
