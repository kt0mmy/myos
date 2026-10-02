#include "window.hpp"
#include "logger.hpp"

Window::Window(int width, int height, PixelFormat shadow_format) : width_{width}, height_{height}
{
    data_.resize(height);
    for (int y = 0; y < height; y++)
    {
        data_[y].resize(width);
    }

    FrameBuferConfig config{};
    config.frame_buffer = nullptr;
    config.horizontal_resolution = width;
    config.vertical_resolution = height;
    config.pixel_format = shadow_format;

    if (auto err = shadow_buffer_.Initialize(config))
    {
        Log(kError, "failed to initialize frame buffer: %s at %s:%d\n", err.Name(), err.File(), err.Line());
    }
};

void Window::DrawTo(FrameBuffer &screen, Vector2D<int> position)
{
    if (!transparent_color_)
    {
        screen.Copy(position, shadow_buffer_);
        return;
    }

    const auto tc = transparent_color_.value();
    auto &writer = screen.Writer();
    for (int dy = 0; dy < Height(); dy++)
    {
        for (int dx = 0; dx < Width(); dx++)
        {
            const auto c = At(dx, dy);
            if (c != tc)
                writer.Write(position.x + dx, position.y + dy, c);
        }
    }
}

void Window::Write(int x, int y, PixelColor c)
{
    data_[x][y] = c;
    shadow_buffer_.Writer().Write(x, y, c);
}

const PixelColor &Window::At(int x, int y) const
{
    return data_[y][x];
}

int Window::Width() const
{
    return width_;
}

int Window::Height() const
{
    return height_;
}

Window::WindowWriter *Window::Writer()
{
    return &writer_;
}

void Window::SetTranparentColor(std::optional<PixelColor> c)
{
    transparent_color_ = c;
}