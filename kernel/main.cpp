#include <cstdint>
#include "frame_buffer_config.hpp"

struct PixelColor {
    uint8_t r, g, b;
};

/**
 * @retval 0   成功
 * @retval 非0 失敗
 */
int WritePixel(const FrameBuferConfig& config, int x, int y, const PixelColor& c) {
    const int pixel_position = config.pixels_per_scan_line * y + x;
    uint8_t* p = &config.frame_buffer[4 * pixel_position];
    
    if (config.pixel_format == kPixelBGRResv8BitPerColor) {
        p[0] = c.b;
        p[1] = c.g;
        p[2] = c.r;
    } else if (config.pixel_format == kPixelRGBResv8BitPerColor) {
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
    } else {
        return -1;
    }

    return 0;
}

// NOTE: マングリングを防ぐ
extern "C" void KernelMain(const FrameBuferConfig& frame_buffer_config) {
    for (int x = 0;x<frame_buffer_config.horizontal_resolution;x++) {
        for (int y=0;y<frame_buffer_config.vertical_resolution;y++) {
            WritePixel(frame_buffer_config, x, y, {0, 0, 255});
        }
    }
    while (1) __asm__("hlt");
}