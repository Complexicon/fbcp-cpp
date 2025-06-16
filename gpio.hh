#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include <unistd.h>

#define GPSET0 7
#define GPSET1 8

#define GPCLR0 10
#define GPCLR1 11

#define GPLEV0 13
#define GPLEV1 14

#define GPPUD 37
#define GPPUDCLK0 38
#define GPPUDCLK1 39

#define PI_INPUT 0
#define PI_OUTPUT 1
#define PI_ALT0 4
#define PI_ALT1 5
#define PI_ALT2 6
#define PI_ALT3 7
#define PI_ALT4 3
#define PI_ALT5 2

#define PI_BANK (pin >> 5)
#define PI_BIT (1 << (pin & 0x1F))

class gpio {
  static volatile inline uint32_t *gpio_reg{};

public:
  static const int HIGH = 1;
  static const int LOW = 1;

  static inline bool init() {

    int gpiomem_fd = open("/dev/gpiomem", O_RDWR | O_SYNC);

    if (gpiomem_fd < 0) {
      std::cerr << "failed to open /dev/gpiomem" << std::endl;
      return false;
    }

    gpio_reg = (uint32_t *)mmap(NULL, 0xB4, PROT_READ | PROT_WRITE, MAP_SHARED,
                                gpiomem_fd, 0);

    close(gpiomem_fd);

    if (gpio_reg == MAP_FAILED) {
      std::cerr << "mmap failed for gpios" << std::endl;;
      return false;
    }

    return true;
  }

  static inline int read(int pin) {
    if ((*(gpio_reg + GPLEV0 + PI_BANK) & PI_BIT) != 0)
      return 1;
    else
      return 0;
  }

  static inline void set(int pin, uint8_t level) {
    if (level == 0)
      *(gpio_reg + GPCLR0 + PI_BANK) = PI_BIT;
    else
      *(gpio_reg + GPSET0 + PI_BANK) = PI_BIT;
  }

  static inline void set_mode(int pin, uint8_t mode) {
    int reg, shift;

    reg = pin / 10;
    shift = (pin % 10) * 3;

    gpio_reg[reg] = (gpio_reg[reg] & ~(7 << shift)) | (mode << shift);
  }
};