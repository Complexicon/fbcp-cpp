#pragma once

#include "display.hh"
// #include "gpio.hh"
#include <pigpio.h>
#include "spi.hh"
#include <memory>
#include <array>

// 18 bits/pixel R6G6B6 format (padded to 3 bytes per pixel), and no 16-bits
// R5G6B5 mode.
#define DISPLAY_COLOR_FORMAT_R6X2G6X2B6X2

#define GPIO_DCX_PIN 25

class ili9488 : public display {

  // Tested with display model ER-TFT035-6 from BuyDisplay.com:
  // https://www.buydisplay.com/serial-spi-3-5-inch-tft-lcd-module-in-320x480-optl-touchscreen-ili9488
  // Using long dupont wires to connect to a Pi Zero, the clock divider was
  // tested down to a value of 12.

  // Data specific to the ILI948X controllers
  const int DISPLAY_SET_CURSOR_X = 0x2A;
  const int DISPLAY_SET_CURSOR_Y = 0x2B;
  const int DISPLAY_WRITE_PIXELS = 0x2C;

  const int DISPLAY_NATIVE_WIDTH = 320;
  const int DISPLAY_NATIVE_HEIGHT = 480;

  const int SPI_BYTESPERPIXEL = 3;

  std::array<std::array<uint8_t, 3>, 65536> rgb565_to_rgb888{};

  void exec_cmd(uint8_t cmd) {
    // gpio::set(GPIO_DCX_PIN, gpio::LOW);
    gpioWrite(GPIO_DCX_PIN, PI_LOW);
    spi::write_byte(cmd);
    gpioWrite(GPIO_DCX_PIN, PI_HIGH);
    // gpio::set(GPIO_DCX_PIN, gpio::HIGH);
  }

  void exec_cmd(uint8_t cmd, const spi_data_t &params) {
    exec_cmd(cmd);
    spi::write(params);
  }

  spi_data_t line = spi_data_t(width() * SPI_BYTESPERPIXEL, 0);

  spi_data_t set_cursor_x = spi_data_t{0, 0, static_cast<unsigned char>((width()-1) >> 8), static_cast<unsigned char>((width()-1) & 0xFF)};
  spi_data_t set_cursor_y = spi_data_t{0, 0, static_cast<unsigned char>((height()-1) >> 8), static_cast<unsigned char>((height()-1) & 0xFF)};

public:

  ili9488() {
    for (uint32_t i = 0; i < 65536; ++i) {
      uint16_t pixel = static_cast<uint16_t>(i);
      uint8_t r = ((pixel >> 8) & 0xF8) | ((pixel >> 13) & 0x07);
      uint8_t g = ((pixel >> 3) & 0xFC) | ((pixel >> 9) & 0x03);
      uint8_t b = ((pixel << 3) & 0xF8) | ((pixel >> 2) & 0x07);
      rgb565_to_rgb888[i] = {r, g, b};
    }
  }

  void init() override {
    // gpio::set_mode(GPIO_DCX_PIN, PI_OUTPUT);

    gpioSetMode(GPIO_DCX_PIN, PI_OUTPUT);

    // 0xE0 - PGAMCTRL Positive Gamma Control
    exec_cmd(0xE0, spi_data_t{0x00, 0x03, 0x09, 0x08, 0x16, 0x0A, 0x3F, 0x78,
                              0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F});
    // 0xE1 - NGAMCTRL Negative Gamma Control
    exec_cmd(0xE1, spi_data_t{0x00, 0x16, 0x19, 0x03, 0x0F, 0x05, 0x32, 0x45,
                              0x46, 0x04, 0x0E, 0x0D, 0x35, 0x37, 0x0F});
    // 0xC0 Power Control 1
    exec_cmd(0xC0, spi_data_t{0x17, 0x15});
    // 0xC1 Power Control 2
    exec_cmd(0xC1, spi_data_t{0x41});
    // 0xC5 VCOM Control
    exec_cmd(0xC5, spi_data_t{0x00, 0x12, 0x80});

    const uint8_t MADCTL_BGR_PIXEL_ORDER = (1 << 3);

    // 0x36 Memory Access Control - sets display rotation.
    exec_cmd(0x36, spi_data_t{MADCTL_BGR_PIXEL_ORDER});

    // 0x3A Interface Pixel Format (bit depth color space)
    exec_cmd(0x3A, spi_data_t{0x66});
    // 0xB0 Interface Mode Control
    exec_cmd(0xB0, spi_data_t{0x80});
    // 0xB1 Frame Rate Control (in Normal Mode/Full Colors)
    exec_cmd(0xB1, spi_data_t{0xA0});

    // 0xB4 Display Inversion Control.
    exec_cmd(0xB4, spi_data_t{0x02});
    // 0x20 Display Inversion OFF.
    exec_cmd(0x20);

    // 0xB6 Display Function Control.
    exec_cmd(0xB6, spi_data_t{0x02, 0x02});
    // 0xE9 Set Image Function.
    exec_cmd(0xE9, spi_data_t{0x00});
    // 0xF7 Adjuist Control 3
    exec_cmd(0xF7, spi_data_t{0xA9, 0x51, 0x2C, 0x82});
    // 0x11 Exit Sleep Mode. (Sleep OUT)
    exec_cmd(0x11);
    usleep(120 * 1000);
    // 0x29 Display ON.
    exec_cmd(0x29);
    // 0x38 Idle Mode OFF.
    exec_cmd(0x38);
    // 0x13 Normal Display Mode ON.
    exec_cmd(0x13);
  }

  void draw(const rgb565_buf_t &fb) override {

    for(int y = 0; y < height(); y++) {
      int yPixel = 0;
      auto pRow = &fb[y * width()];
      for(int x = 0; x < width(); x++) {
        auto rgb = rgb565_to_rgb888[pRow[x]];
        line[yPixel++] = rgb[0];
        line[yPixel++] = rgb[1];
        line[yPixel++] = rgb[2];
      }

      exec_cmd(DISPLAY_SET_CURSOR_X, set_cursor_x);
      set_cursor_y[0] = (uint8_t)(y >> 8);
      set_cursor_y[1] = (uint8_t)(y & 0xFF);
      exec_cmd(DISPLAY_SET_CURSOR_Y, set_cursor_y);
      exec_cmd(DISPLAY_WRITE_PIXELS, line);
    }

  }

  constexpr int width() override { return DISPLAY_NATIVE_WIDTH; }
  constexpr int height() override { return DISPLAY_NATIVE_HEIGHT; }
};

static inline std::unique_ptr<display> make_display() {
  return std::make_unique<ili9488>();
}