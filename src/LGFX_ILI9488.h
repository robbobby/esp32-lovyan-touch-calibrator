#pragma once
#include <LovyanGFX.hpp>
#include "screen_config_3.5_ILI9488.h"

// Defines from your settings
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15
#define TFT_DC   4
#define TFT_RST  2
#define TOUCH_CS 5
#define TOUCH_IRQ 34

#define SPI_FREQUENCY       27000000
#define SPI_TOUCH_FREQUENCY 2500000

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9488 _panel;
    lgfx::Bus_SPI       _bus;
    lgfx::Light_PWM     _light;
    lgfx::Touch_XPT2046 _touch;

public:
    LGFX(void) {
        { // BUS
            auto cfg = _bus.config();
            cfg.spi_host   = VSPI_HOST;
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
            cfg.pin_bl = -1;
            cfg.freq   = 44100;
            cfg.pwm_channel = 7;
            _light.config(cfg);
            _panel.setLight(&_light);
        }

        { // TOUCH
            auto cfg = _touch.config();
            cfg.spi_host = VSPI_HOST;
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

