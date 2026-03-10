# UniProger — Universal Professional Programmer-Analyzer

<p align="center">
<b>Масштабована програмно-апаратна платформа для програмування, зчитування, клонування та аналізу мікросхем</b>
</p>

---

## 🎯 Можливості

| Функція | Опис |
|---------|------|
| **Програмування** | Запис прошивок у Flash-пам'ять та мікроконтролери |
| **Зчитування** | Дамп вмісту мікросхем пам'яті та MCU |
| **Верифікація** | Порівняння записаних даних з оригіналом |
| **Стирання** | Секторне та повне стирання Flash/MCU |
| **Аналіз** | Сніфер протоколів SPI/I2C/UART/JTAG/SWD |
| **Клонування** | Копіювання вмісту між однотипними чіпами |
| **Емуляція** | Імітація цільового пристрою (розширюється) |

## 🔧 Підтримувані пристрої (MVP)

### Мікросхеми пам'яті
- **SPI Flash**: W25Q16/32/64/128/256, GD25Q64/128, MX25L6433, AT25SF081
- **I2C EEPROM**: 24C01–24C512 та сумісні

### Мікроконтролери
- **AVR (ISP)**: ATmega328P, ATmega32/A, ATmega168, ATmega8, ATmega2560, ATtiny25/45/85
- **STM32 (SWD)**: STM32F1xx та сумісні ARM Cortex-M

### Протоколи
- SPI (Mode 0-3, до 62.5 MHz)
- I2C (Standard/Fast/Fast+)
- JTAG (TAP state machine, chain detection)
- SWD (ARM Serial Wire Debug)
- UART (passthrough, sniffer)
- 1-Wire (Dallas/Maxim)

## 🏗️ Архітектура

```
┌──────────────────────────────────────────────────┐
│              CLI Shell  (USB CDC)                │
├──────────────────────────────────────────────────┤
│           Command Processor                      │
├──────────────────────────────────────────────────┤
│     Device Drivers (Plugin Registry)             │
│   ┌──────────┬──────────┬────────┬──────────┐    │
│   │ SPI Flash│I2C EEPROM│AVR ISP │ STM32 SWD│    │
│   └──────────┴──────────┴────────┴──────────┘    │
├──────────────────────────────────────────────────┤
│        Protocol Engines                          │
│   ┌─────┬─────┬──────┬─────┬──────┬────────┐     │
│   │ SPI │ I2C │ JTAG │ SWD │ UART │ 1-Wire │     │
│   └─────┴─────┴──────┴─────┴──────┴────────┘     │
├──────────────────────────────────────────────────┤
│     HAL (Hardware Abstraction Layer)             │
├──────────────────────────────────────────────────┤
│  Platform: RP2040 │ STM32 │ ESP32 │ RP2350       │
└──────────────────────────────────────────────────┘
```

**Ключова перевага**: заміна платформи потребує лише реалізації HAL-шару (~7 файлів), весь core-код залишається незмінним.

## 📦 Збірка

### Вимоги
**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential python3 g++
```

**Fedora:**
```bash
sudo dnf install cmake arm-none-eabi-gcc-cs arm-none-eabi-newlib gcc-c++ python3
```
- CMake ≥ 3.13
- ARM GCC toolchain (`arm-none-eabi-gcc`)
- [Pico SDK](https://github.com/raspberrypi/pico-sdk) (для RP2040)

### Компіляція для RP2040

```bash
# Встановіть змінну середовища
export PICO_SDK_PATH=/path/to/pico-sdk

# Створіть директорію збірки
mkdir build && cd build

# Конфігуруйте
cmake -DUNIPROGER_PLATFORM=rp2040 ..

# Зберіть
make -j$(nproc)

# Результат: build/uniproger.uf2
```

### Прошивка

1. Затисніть кнопку BOOTSEL на Pico
2. Підключіть USB
3. Скопіюйте файл:
```bash
cp build/uniproger.uf2 /media/$USER/RPI-RP2/
```

## 🔌 Розводка контактів (Raspberry Pi Pico)

| Функція | GPIO | Пін Pico |
|---------|------|----------|
| **SPI SCK** | GP2 | Pin 4 |
| **SPI MOSI** | GP3 | Pin 5 |
| **SPI MISO** | GP4 | Pin 6 |
| **SPI CS** | GP5 | Pin 7 |
| **I2C SDA** | GP0 | Pin 1 |
| **I2C SCL** | GP1 | Pin 2 |
| **JTAG TCK / SWD SWCLK** | GP18 | Pin 24 |
| **JTAG TMS / SWD SWDIO** | GP19 | Pin 25 |
| **JTAG TDI** | GP20 | Pin 26 |
| **JTAG TDO** | GP21 | Pin 27 |
| **nRESET** | GP22 | Pin 29 |
| **AVR RST** | GP6 | Pin 9 |
| **1-Wire** | GP7 | Pin 10 |
| **UART TX** | GP8 | Pin 11 |
| **UART RX** | GP9 | Pin 12 |
| **VCC Enable** | GP26 | Pin 31 |

## 💻 CLI команди

```
up> help
  detect       Detect connected devices
  info         Show system information
  scan         Scan I2C bus for devices
  pin          GPIO pin control
  reset        Reset system
  help         Show available commands
  version      Show firmware version
```

## 🔌 Додавання нового пристрою

1. Створіть файл драйвера в `src/core/device/`:

```c
#include "device.h"

static up_status_t my_device_detect(up_device_t *dev) { /* ... */ }
static up_status_t my_device_read(up_device_t *dev, ...) { /* ... */ }
// ...

const up_device_ops_t my_device_ops = {
    .name    = "my_device",
    .type    = UP_DEVICE_TYPE_GENERIC,
    .detect  = my_device_detect,
    .read    = my_device_read,
    // ...
};
```

2. Зареєструйте в `main.c`:
```c
extern const up_device_ops_t my_device_ops;
up_device_register(&my_device_ops);
```

## 🔄 Портування на іншу платформу

1. Створіть директорію `src/platform/<platform>/`
2. Реалізуйте HAL-функції:
   - `hal_gpio_*` — GPIO управління
   - `hal_spi_*` — SPI периферія
   - `hal_i2c_*` — I2C периферія  
   - `hal_uart_*` — UART периферія
   - `hal_timer_*` — Таймери та затримки
   - `hal_pio_*` — PIO/bitbang (опціонально)
   - `hal_platform_*` — Ініціалізація платформи
3. Додайте в `CMakeLists.txt` нову секцію для платформи

## 📄 Ліцензія

MIT License — вільне використання у будь-яких проєктах.
