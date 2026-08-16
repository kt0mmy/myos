#include <cstdint>
#include <cstddef>
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"


void *operator new(size_t size, void *buf)
{
    return buf;
}

void operator delete(void *obj) noexcept {}

/**
 * @retval 0   成功
 * @retval 非0 失敗
 */
int WritePixel(const FrameBuferConfig &config, int x, int y, const PixelColor &c)
{
    const int pixel_position = config.pixels_per_scan_line * y + x;
    uint8_t *p = &config.frame_buffer[4 * pixel_position];

    if (config.pixel_format == kPixelBGRResv8BitPerColor)
    {
        p[0] = c.b;
        p[1] = c.g;
        p[2] = c.r;
    }
    else if (config.pixel_format == kPixelRGBResv8BitPerColor)
    {
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
    }
    else
    {
        return -1;
    }

    return 0;
}

// class のサイズ？
char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter *pixel_writer;

// NOTE: マングリングを防ぐ
extern "C" void KernelMain(const FrameBuferConfig &frame_buffer_config)
{
    switch (frame_buffer_config.pixel_format)
    {
    case kPixelBGRResv8BitPerColor:
        pixel_writer = new (pixel_writer_buf) BGRResv8BitPerColorPixelWriter{frame_buffer_config};
        break;
    case kPixelRGBResv8BitPerColor:
        pixel_writer = new (pixel_writer_buf) RGBResv8BitPerColorPixelWriter{frame_buffer_config};
        break;
    }
    for (int x = 0; x < frame_buffer_config.horizontal_resolution; x++)
    {
        for (int y = 0; y < frame_buffer_config.vertical_resolution; y++)
        {
            pixel_writer->Write(x, y, {255, 0, 255});
        }
    }

    WriteAscii(*pixel_writer, 50, 50, 'A', {0, 0, 0});
    WriteAscii(*pixel_writer, 58, 50, 'A', {0, 0, 0});
    while (1)
        __asm__("hlt");
}