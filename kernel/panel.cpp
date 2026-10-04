#include <cstdint>
#include "panel.hpp"
#include "font.hpp"
namespace
{
    const int kCloseButtonWidth = 16;
    const int kCloseButtonHeight = 14;
    const char close_button[kCloseButtonHeight][kCloseButtonWidth + 1] = {
        "...............@",
        ".:::::::::::::$@",
        ".:::::::::::::$@",
        ".:::@@::::@@::$@",
        ".::::@@::@@:::$@",
        ".:::::@@@@::::$@",
        ".::::::@@:::::$@",
        ".:::::@@@@::::$@",
        ".::::@@::@@:::$@",
        ".:::@@::::@@::$@",
        ".:::::::::::::$@",
        ".:::::::::::::$@",
        ".$$$$$$$$$$$$$$@",
        "@@@@@@@@@@@@@@@@",
    };

    constexpr PixelColor ToColor(uint32_t c)
    {
        return {
            static_cast<uint8_t>((c >> 16) & 0xff),
            static_cast<uint8_t>((c >> 8) & 0xff),
            static_cast<uint8_t>(c & 0xff),
        };
    }
}

void DrawPanel(PixelWriter &writer, const char *title)
{
    auto fill_rect = [&writer](Vector2D<int> pos, Vector2D<int> size, uint32_t c)
    {
        FillRectangle(writer, pos, size, ToColor(c));
    };

    const auto panel_w = writer.Width();
    const auto panel_h = writer.Height();

    fill_rect({0, 0}, {panel_w, 1}, 0xc6c6c6);
    fill_rect({1, 1}, {panel_w - 2, 1}, 0xffffff);
    fill_rect({0, 0}, {1, panel_h}, 0xc6c6c6);
    fill_rect({1, 1}, {1, panel_h - 2}, 0xffffff);
    fill_rect({panel_w - 2, 1}, {1, panel_h - 2}, 0x848484);
    fill_rect({panel_w - 1, 0}, {1, panel_h}, 0x000000);
    fill_rect({2, 2}, {panel_w - 4, panel_h - 4}, 0xc6c6c6);
    fill_rect({3, 3}, {panel_w - 6, 18}, 0x000084);
    fill_rect({1, panel_h - 2}, {panel_w - 2, 1}, 0x848484);
    fill_rect({0, panel_h - 1}, {panel_w, 1}, 0x000000);

    WriteString(writer, {24, 4}, title, ToColor(0xffffff));

    for (int y = 0; y < kCloseButtonHeight; ++y)
    {
        for (int x = 0; x < kCloseButtonWidth; ++x)
        {
            PixelColor c = ToColor(0xffffff);
            if (close_button[y][x] == '@')
            {
                c = ToColor(0x000000);
            }
            else if (close_button[y][x] == '$')
            {
                c = ToColor(0x848484);
            }
            else if (close_button[y][x] == ':')
            {
                c = ToColor(0xc6c6c6);
            }
            writer.Write({panel_w - 5 - kCloseButtonWidth + x, 5 + y}, c);
        }
    }
}