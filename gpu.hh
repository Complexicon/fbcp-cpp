#pragma once

#include <bcm_host.h>
#include <iostream>
#include <vector>

class gpu {

  static inline DISPMANX_RESOURCE_HANDLE_T screen_resource{};
  static inline VC_RECT_T rect{};
  static inline DISPMANX_DISPLAY_HANDLE_T display{};
  static inline DISPMANX_MODEINFO_T display_info;

  static inline std::vector<uint16_t> m_framebuffer{};
  static inline int pitch{};

public:
  static inline bool init() {
    bcm_host_init();
    display = vc_dispmanx_display_open(0);

    if (!display) {
      std::cerr << "vc_dispmanx_display_open failed! Make sure to have "
                   "hdmi_force_hotplug=1 setting in /boot/config.txt"
                << std::endl;
      return false;
    }

    if (vc_dispmanx_display_get_info(display, &display_info)) {
      std::cerr << "vc_dispmanx_display_get_info failed!" << std::endl;
      return false;
    }

    return true;
  }

  static inline int32_t fb_width() {
    return display_info.width;
  }

  static inline int32_t fb_height() {
    return display_info.height;
  }

  static inline bool set_capture_rect(int width, int height) {
    uint32_t image_prt;
    screen_resource = vc_dispmanx_resource_create(VC_IMAGE_RGB565, width, height, &image_prt);
    
    if (vc_dispmanx_rect_set(&rect, 0, 0, width, height)) {
      std::cerr << "vc_dispmanx_rect_set failed!" << std::endl;
      return false;
    }

    pitch = VCOS_ALIGN_UP(width * sizeof(uint16_t), 32);

    m_framebuffer.resize((pitch * height) / sizeof(uint16_t));

    return true;
  }

  static inline void update_framebuffer() {
    if (vc_dispmanx_snapshot(display, screen_resource, (DISPMANX_TRANSFORM_T)0)) {
        std::cerr << "vc_dispmanx_snapshot failed!" << std::endl;
      }
    
      if(vc_dispmanx_resource_read_data(screen_resource, &rect, m_framebuffer.data(), pitch)) {
        std::cerr << "vc_dispmanx_snapshot failed!" << std::endl;
      }
  }

  static inline const std::vector<uint16_t>& framebuffer() {
    return m_framebuffer;
  }

};