// #include "gpio.hh"
#include <pigpio.h>
#include "spi.hh"
#include "display.hh"
#include "gpu.hh"

int main() {

  if(gpioInitialise() < 0) {
    return 1;
  }


  // if(!gpio::init()) {
  //   return 1;
  // }

  if(!spi::setup("/dev/spidev0.0", SPI_MODE_0, 8, 50'000'000)) {
    return 1;
  }

  auto disp = make_display();
  disp->init();

  if(!gpu::init()) {
    return 1;
  }
  
  gpu::set_capture_rect(disp->width(), disp->height());

  while(true) {
    gpu::update_framebuffer();
    disp->draw(gpu::framebuffer());
  }

}