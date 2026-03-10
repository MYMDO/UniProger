cmake_minimum_required(VERSION 3.13)

# ── Platform selection ──────────────────────────────────────────────────
# Usage: cmake -DUNIPROGER_PLATFORM=rp2040 ..
# Supported: rp2040 (default), stm32, esp32
if(NOT DEFINED UNIPROGER_PLATFORM)
    set(UNIPROGER_PLATFORM "rp2040" CACHE STRING "Target platform")
endif()

message(STATUS "UniProger platform: ${UNIPROGER_PLATFORM}")

# ── Pico SDK bootstrap (must come before project()) ────────────────────
if(UNIPROGER_PLATFORM STREQUAL "rp2040")
    if(DEFINED ENV{PICO_SDK_PATH})
        set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    elseif(NOT DEFINED PICO_SDK_PATH)
        message(FATAL_ERROR "PICO_SDK_PATH not set. Set via env or -DPICO_SDK_PATH=...")
    endif()
    include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
endif()

# ── Project definition ──────────────────────────────────────────────────
project(uniproger
    VERSION 0.1.0
    DESCRIPTION "Universal Professional Programmer-Analyzer"
    LANGUAGES C CXX ASM
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── Pico SDK init ───────────────────────────────────────────────────────
if(UNIPROGER_PLATFORM STREQUAL "rp2040")
    pico_sdk_init()
endif()

# ── Source files ────────────────────────────────────────────────────────

# Core (platform-independent)
set(CORE_SOURCES
    src/core/device/device_registry.c
    src/core/device/memory/spi_flash.c
    src/core/device/memory/i2c_eeprom.c
    src/core/device/mcu/avr_isp.c
    src/core/device/mcu/stm32_swd.c
    src/core/protocol/spi_protocol.c
    src/core/protocol/i2c_protocol.c
    src/core/protocol/jtag_protocol.c
    src/core/protocol/swd_protocol.c
    src/core/protocol/uart_protocol.c
    src/core/protocol/onewire_protocol.c
    src/core/buffer/buffer.c
    src/core/utils/crc.c
    src/core/utils/log.c
)

# CLI
set(CLI_SOURCES
    src/cli/cli.c
    src/cli/commands.c
)

# Platform-specific sources
if(UNIPROGER_PLATFORM STREQUAL "rp2040")
    set(PLATFORM_SOURCES
        src/platform/rp2040/rp2040_platform.c
        src/platform/rp2040/rp2040_hal_gpio.c
        src/platform/rp2040/rp2040_hal_spi.c
        src/platform/rp2040/rp2040_hal_i2c.c
        src/platform/rp2040/rp2040_hal_uart.c
        src/platform/rp2040/rp2040_hal_timer.c
        src/platform/rp2040/rp2040_hal_pio.c
    )
endif()

# ── Executable ──────────────────────────────────────────────────────────
add_executable(uniproger
    src/main.c
    ${CORE_SOURCES}
    ${CLI_SOURCES}
    ${PLATFORM_SOURCES}
)

# ── Include paths ───────────────────────────────────────────────────────
target_include_directories(uniproger PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/core/hal
    ${CMAKE_SOURCE_DIR}/src/core/protocol
    ${CMAKE_SOURCE_DIR}/src/core/device
    ${CMAKE_SOURCE_DIR}/src/core/buffer
    ${CMAKE_SOURCE_DIR}/src/core/utils
    ${CMAKE_SOURCE_DIR}/src/cli
)

# ── Platform-specific configuration ────────────────────────────────────
if(UNIPROGER_PLATFORM STREQUAL "rp2040")
    target_compile_definitions(uniproger PRIVATE
        UNIPROGER_PLATFORM_RP2040=1
    )

    target_include_directories(uniproger PRIVATE
        ${CMAKE_SOURCE_DIR}/src/platform/rp2040
    )

    # PIO programs
    pico_generate_pio_header(uniproger
        ${CMAKE_SOURCE_DIR}/src/platform/rp2040/pio/jtag.pio
    )
    pico_generate_pio_header(uniproger
        ${CMAKE_SOURCE_DIR}/src/platform/rp2040/pio/swd.pio
    )

    # Pico SDK libraries
    target_link_libraries(uniproger PRIVATE
        pico_stdlib
        pico_unique_id
        hardware_spi
        hardware_i2c
        hardware_uart
        hardware_timer
        hardware_gpio
        hardware_dma
        hardware_pio
        hardware_clocks
        hardware_watchdog
        tinyusb_device
        tinyusb_board
    )

    # Enable USB output, disable UART output for stdio
    pico_enable_stdio_usb(uniproger 1)
    pico_enable_stdio_uart(uniproger 0)

    # Generate UF2 file
    pico_add_extra_outputs(uniproger)

    message(STATUS "RP2040 build. Flash with: cp build/uniproger.uf2 /media/<user>/RPI-RP2")
endif()

# ── Compiler warnings ──────────────────────────────────────────────────
target_compile_options(uniproger PRIVATE
    -Wall
    -Wextra
    -Werror=return-type
    -Wno-unused-parameter
    -Wno-unused-function
    -O2
)

# ── Version info ────────────────────────────────────────────────────────
target_compile_definitions(uniproger PRIVATE
    UNIPROGER_VERSION="${PROJECT_VERSION}"
)

message(STATUS "UniProger v${PROJECT_VERSION} configured for ${UNIPROGER_PLATFORM}")
