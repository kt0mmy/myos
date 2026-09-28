#include "window.hpp"

Window::Window(int width, int height) : width_{width}, height_{height}
{
    data_.resize(height);
    for (int y = 0; y < height; y++)
    {
        data_[y].resize(width);
    }
};

void Window::DrawTo(PixelWriter &writer, Vector2D<int> position)
{
    if (!transparent_color_)
    {
        for (int dy = 0; dy < Height(); dy++)
        {
            for (int dx = 0; dx < Width(); dx++)
            {
                writer.Write(position.x + dx, position.y + dy, At(dx, dy));
            }
        }
        return;
    }
    const auto tc = transparent_color_.value();
    for (int dy = 0; dy < Height(); dy++)
    {
        for (int dx = 0; dx < Width(); dx++)
        {
            const auto c = At(dx, dy);
            if (c != tc)
                writer.Write(position.x + dx, position.y + dy, At(dx, dy));
        }
    }
}

PixelColor &Window::At(int x, int y)
{
    return data_[y][x];
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