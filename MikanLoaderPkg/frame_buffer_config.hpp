#pragma once
#include <stdint.h>

enum PixelFormat
{
    // 24 bit だが、メモリ上は 32bit で 1pixel を表現する
    kPixelRGBResv8BitPerColor,
    kPixelBGRResv8BitPerColor,
};

/**
 * (x, y) := 4 * (y * pixels_per_scan_line + x)
 * (32bit)(32bit)   ・・・       (非表示領域) | (32bit)(32bit)   ・・・       (非表示領域) | ・・・ | (非表示領域)     ・・・       (非表示領域)
 */
struct FrameBuferConfig {
    uint8_t* frame_buffer;
    uint32_t pixels_per_scan_line;
    uint32_t horizontal_resolution;
    uint32_t vertical_resolution;
    enum PixelFormat pixel_format;
};