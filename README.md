# Touch Screen Calibration Script

A standalone PlatformIO project for calibrating resistive touch screens on ESP32. Uses LovyanGFX for display control and LVGL for the UI. Automatically runs calibration on startup and provides a grid of 50 buttons to test touch accuracy across the entire screen.

## What This Does

This script helps you:
1. Calibrate your touch screen to ensure accurate touch detection
2. Test touch accuracy across the entire screen with 50 test buttons
3. Get the 8 calibration values needed for your project
4. Easily adapt to different screen sizes and configurations

## Prerequisites

- **PlatformIO** installed (VS Code extension or CLI)
- **ESP32 development board** (tested on ESP32-DevKit)
- **Touch screen** with resistive touch controller (XPT2046)
- **Display** compatible with LovyanGFX (ILI9488, ST7789, ILI9341, etc.)

## Features

- Automatic calibration on startup (with skip option if values exist)
- 50 test buttons covering the entire screen
- Toggle calibration values display (Button 49 - VIEW)
- Recalibrate anytime (Button 50 - CAL)
- Screen-specific configuration files for easy adaptation

## Quick Start

1. **Clone or download this project**

2. **Configure your display**:
   - Edit `src/screen_config_3.5_ILI9488.h`:
     - Set `SCREEN_WIDTH`, `SCREEN_HEIGHT`, `SCREEN_DEFAULT_ROTATION`
   - Edit `src/LGFX_ILI9488.h`:
     - Update pin definitions (MISO, MOSI, SCLK, CS, DC, RST, TOUCH_CS, TOUCH_IRQ)
     - Change panel type if different (default: ILI9488)
   - Edit `platformio.ini`:
     - Update build flags with your pin numbers if different

3. **Build and upload**:
   ```bash
   npm run calibrate
   # or
   pio run --target upload
   ```

4. **Calibrate**:
   - On startup: Touch anywhere within 5 seconds to skip (if calibration values exist), or wait to start calibration
   - Follow on-screen prompts to touch each corner
   - **Important**: Run 3-5 calibrations for best accuracy (see Calibration Process below)

5. **View and save values**:
   - Press VIEW button (49) to toggle calibration values display
   - Copy the 8 values shown
   - Paste them into `TOUCH_CAL_VALUES` in your screen config file
   - Rebuild and upload to make calibration permanent

## Adapting for Different Screens

If you have a different display size or model, follow these steps:

### Step 1: Create Screen Config File

Create a new file `src/screen_config_{your_display_name}.h` (e.g., `screen_config_2.8_ST7789.h`):

```cpp
#pragma once

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 480
#define SCREEN_DEFAULT_ROTATION 1
#define LVGL_BUFFER_SIZE (SCREEN_WIDTH * 10)

#define TOUCH_X_MIN 0
#define TOUCH_X_MAX (SCREEN_WIDTH - 1)
#define TOUCH_Y_MIN 0
#define TOUCH_Y_MAX (SCREEN_HEIGHT - 1)

static const uint16_t TOUCH_CAL_VALUES[8] = {0, 0, 0, 0, 0, 0, 0, 0};
```

### Step 2: Update main.cpp

Change line 5 in `src/main.cpp`:
```cpp
#include "screen_config_3.5_ILI9488.h"  // Change this
```
to:
```cpp
#include "screen_config_{your_display_name}.h"  // Your new file
```

### Step 3: Update Display Driver

Edit `src/LGFX_ILI9488.h`:

**Change panel type** (if different from ILI9488):
```cpp
lgfx::Panel_ST7789 _panel;  // or Panel_ILI9341, Panel_GC9A01, etc.
```

**Update pin definitions** at the top of the file:
```cpp
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15
#define TFT_DC   4
#define TFT_RST  2
#define TOUCH_CS 5
#define TOUCH_IRQ 34
```

**Update SPI frequencies** if needed:
```cpp
#define SPI_FREQUENCY       27000000
#define SPI_TOUCH_FREQUENCY 2500000
```

### Step 4: Update platformio.ini (Optional)

If your pin numbers differ from the defaults, update the build flags:
```ini
build_flags =
    -D TFT_MISO=19
    -D TFT_MOSI=23
    -D TFT_SCLK=18
    -D TFT_CS=15
    -D TFT_DC=4
    -D TFT_RST=2
    -D TOUCH_CS=5
    -D TOUCH_IRQ=34
    -D SPI_FREQUENCY=27000000
    -D SPI_TOUCH_FREQUENCY=2500000
```

**Note**: The pin definitions in `LGFX_ILI9488.h` take precedence, but these build flags can be useful for conditional compilation.

## Calibration Process

### Multiple Tests for Accuracy

⚠️ **Run 3-5 calibration tests for best results**

1. Run calibration multiple times (press CAL button)
2. After each test, press VIEW to see values
3. Record all values from each test
4. Use extreme values:
   - **Low indices (0, 2, 3, 7)**: Take the **LOWEST** value
   - **High indices (1, 4, 5, 6)**: Take the **HIGHEST** value

**Example:**
```
Test 1: {246, 3849, 262, 234, 3945, 3889, 3994, 240}
Test 2: {253, 3785, 257, 189, 3922, 3851, 3897, 194}
Test 3: {250, 3850, 260, 200, 3930, 3870, 3980, 220}

Final: {246, 3850, 257, 189, 3945, 3889, 3994, 194}
       (lowest) (highest) (lowest) (lowest) (highest) (highest) (highest) (lowest)
```

### Saving Calibration Values

1. Press VIEW button to display values
2. Copy the 8 values shown
3. Update `TOUCH_CAL_VALUES` in your screen config file
4. Rebuild and upload

## File Structure

```
calibration_script/
├── src/
│   ├── main.cpp                    # Main calibration script
│   ├── LGFX_ILI9488.h             # Display driver config
│   ├── LGFX_ILI9488.cpp            # Display instance
│   └── screen_config_*.h           # Screen-specific configs
├── include/
│   └── lv_conf.h                   # LVGL configuration
├── platformio.ini                  # Project configuration
└── README.md                       # This file
```

## Tips

- Use a stylus for precise corner touches
- Touch as close to screen edges as possible
- Test all corner buttons (1, 5, 46, 50) after calibration
- VIEW button toggles values on/off - won't block testing

## Troubleshooting

- **Touch doesn't work**: Check pin connections in `LGFX_ILI9488.h`
- **Calibration seems off**: Run more tests, use extreme values method
- **Buttons don't respond**: Verify calibration completed (check VIEW button)

## Dependencies

- PlatformIO
- LovyanGFX @ ^1.1.10
- LVGL @ ^9.1.0
- ESP32 development board
