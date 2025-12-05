#pragma once

// Screen configuration for 3.5 inch ILI9488 display
// Change this file when switching to a different screen

// Screen dimensions (native resolution)
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 480

// Default rotation (0-3, where 0 is portrait)
#define SCREEN_DEFAULT_ROTATION 1

// LVGL buffer size (1/10 of screen size for partial rendering)
// Formula: (SCREEN_WIDTH * SCREEN_HEIGHT / 10)
#define LVGL_BUFFER_SIZE (SCREEN_WIDTH * 10)

// Touch controller initial bounds (before calibration)
// These are typically 0 to (width-1) and 0 to (height-1)
#define TOUCH_X_MIN 0
#define TOUCH_X_MAX (SCREEN_WIDTH - 1)
#define TOUCH_Y_MIN 0
#define TOUCH_Y_MAX (SCREEN_HEIGHT - 1)

// Touch calibration values
// These values are obtained from LovyanGFX calibrateTouch() routine
// Format: uint16_t array of 8 values
// static const uint16_t TOUCH_CAL_VALUES[8] = {253, 3785, 257, 189, 3922, 3851, 3897, 194};
// static const uint16_t TOUCH_CAL_VALUES[8] = {246, 3849, 262, 234, 3945, 3889, 3994, 240};
// Take a few readings, where the numbers are low, take the lowest number, numbers are high take the highest
// This way you will calibrate best possible as close to the edge/corners as possible
static const uint16_t TOUCH_CAL_VALUES[8] = {246, 3849, 257, 189, 3945, 3889, 3994, 194};

