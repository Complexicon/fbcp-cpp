#pragma once

#include <cstdint>
#include <vector>
#include <memory>

typedef std::vector<uint16_t> rgb565_buf_t;

class display {
    public:
    virtual ~display() {};
    virtual void init() = 0;
    virtual void draw(const rgb565_buf_t& buffer) = 0;
    virtual constexpr int width() = 0;
    virtual constexpr int height() = 0;
};

static inline std::unique_ptr<display> make_display();

#ifdef DISPLAY_ILI9488
#include "ili9488.hh"
#endif