#pragma once
#include <LovyanGFX.hpp>
#include "screen_config_4.0_ST7796S.h"

// Pin definitions - adjust these to match your wiring (can be overridden by -D flags)
// Note: ESP32-S3 GPIO range is 0-48 (GPIO 19-20, 22-25 are typically USB)
#ifndef TFT_MISO
#define TFT_MISO 13
#endif
#ifndef TFT_MOSI
#define TFT_MOSI 11
#endif
#ifndef TFT_SCLK
#define TFT_SCLK 12
#endif
#ifndef TFT_CS
#define TFT_CS   10
#endif
#ifndef TFT_DC
#define TFT_DC   9
#endif
#ifndef TFT_RST
#define TFT_RST  8
#endif
#ifndef TOUCH_CS
#define TOUCH_CS 6
#endif
#ifndef TOUCH_IRQ
#define TOUCH_IRQ 5
#endif
#ifndef BACKLIGHT_PIN
#define BACKLIGHT_PIN 7
#endif

#ifndef SPI_FREQUENCY
#define SPI_FREQUENCY       27000000
#endif
#ifndef SPI_TOUCH_FREQUENCY
#define SPI_TOUCH_FREQUENCY 2500000
#endif

#ifndef VSPI_HOST
#define VSPI_HOST SPI2_HOST
#endif

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7796 _panel;
    lgfx::Bus_SPI       _bus;
    lgfx::Light_PWM     _light;
    lgfx::Touch_XPT2046 _touch;

public:
    LGFX(void) {
        { // BUS
            auto cfg = _bus.config();
            cfg.spi_host   = VSPI_HOST;  // ESP32-S3 uses SPI2_HOST or SPI3_HOST
            cfg.spi_mode   = 0;
            cfg.freq_write = SPI_FREQUENCY;
            cfg.freq_read  = 16000000;
            cfg.pin_sclk = TFT_SCLK;
            cfg.pin_mosi = TFT_MOSI;
            cfg.pin_miso = TFT_MISO;
            cfg.pin_dc   = TFT_DC;
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            _bus.config(cfg);
            _panel.setBus(&_bus);
        }

        { // PANEL
            auto cfg = _panel.config();
            cfg.pin_cs   = TFT_CS;
            cfg.pin_rst  = TFT_RST;
            cfg.pin_busy = -1;
            cfg.panel_width  = SCREEN_WIDTH;
            cfg.panel_height = SCREEN_HEIGHT;
            cfg.bus_shared = true;
            cfg.readable = false;
            _panel.config(cfg);
        }

        { // BACKLIGHT
            auto cfg = _light.config();
            cfg.pin_bl = BACKLIGHT_PIN;
            cfg.freq   = 44100;
            cfg.pwm_channel = 7;
            _light.config(cfg);
            _panel.setLight(&_light);
        }

        { // TOUCH
            auto cfg = _touch.config();
            cfg.spi_host = VSPI_HOST;  // ESP32-S3 uses SPI2_HOST or SPI3_HOST
            cfg.freq = SPI_TOUCH_FREQUENCY;
            cfg.pin_sclk = TFT_SCLK;
            cfg.pin_mosi = TFT_MOSI;
            cfg.pin_miso = TFT_MISO;
            cfg.pin_cs   = TOUCH_CS;
            cfg.pin_int  = TOUCH_IRQ;
            cfg.bus_shared = true;
            cfg.x_min = TOUCH_X_MIN;
            cfg.x_max = TOUCH_X_MAX;
            cfg.y_min = TOUCH_Y_MIN;
            cfg.y_max = TOUCH_Y_MAX;
            _touch.config(cfg);
            _panel.setTouch(&_touch);
        }

        setPanel(&_panel);
    }
};

extern LGFX display;
