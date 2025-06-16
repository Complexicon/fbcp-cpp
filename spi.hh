#pragma once

#include <cerrno>
#include <cstdint>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <iostream>

typedef std::vector<uint8_t> spi_data_t;

class spi {

  static inline int spidev_fd{-1};

  static inline struct spi_ioc_transfer xfer { 0 };

public:
  static inline bool setup(std::string device_path, uint8_t spi_mode,
                           uint8_t bits_per_word, uint32_t max_spi_freq) {

    spidev_fd = open(device_path.c_str(), O_RDWR);
    if (spidev_fd < 0) {
      std::cerr << "failed to open spi device" << std::endl;
      return false;
    }

    if (ioctl(spidev_fd, SPI_IOC_WR_MODE, &spi_mode) == -1) {
      std::cerr << "Failed to set SPI mode" << std::endl;
      close(spidev_fd);
      return false;
    }

    if (ioctl(spidev_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) == -1) {
      std::cerr << "Failed to set bits per word" << std::endl;
      close(spidev_fd);
      return false;
    }

    xfer.bits_per_word = bits_per_word;

    if (ioctl(spidev_fd, SPI_IOC_WR_MAX_SPEED_HZ, &max_spi_freq) == -1) {
      std::cerr << "Failed to set max speed" << std::endl;
      close(spidev_fd);
      return false;
    }

    xfer.speed_hz = max_spi_freq;

    return true;
  }

  static inline void write(const spi_data_t &data) {
    xfer.tx_buf = (unsigned long)data.data();
    xfer.len = (uint32_t)data.size();
    if (ioctl(spidev_fd, SPI_IOC_MESSAGE(1), &xfer) == -1) {
      std::cerr << "write SPI transfer failed" << errno  << std::endl;
      close(spidev_fd);
      exit(1);
    }
  }

  static inline void write_byte(uint8_t byte) {
    xfer.tx_buf = (unsigned long)&byte;
    xfer.len = 1;
    if (ioctl(spidev_fd, SPI_IOC_MESSAGE(1), &xfer) == -1) {
      std::cerr << "write_byte SPI transfer failed" << errno  << std::endl;
      close(spidev_fd);
      exit(1);
    }
  }
};