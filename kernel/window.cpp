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
    Log(kError, "failed to initialize shadowbuffer: %s at %s:%d\n", err.Name(), err.File(), err.Line());
  }
}

void Window::DrawTo(FrameBuffer &screen, Vector2D<int> position)
{
  if (!transparent_color_)
  {
    screen.Copy(position, shadow_buffer_);
    return;
  }

  const auto tc = transparent_color_.value();
  auto &writer = screen.Writer();

  for (int dy = std::max(0, 0 - position.y); dy < std::min(Height(), writer.Height() - position.y); dy++)
  {
    for (int dx = std::max(0, 0 - position.x); dx < std::min(Width(), writer.Width() - position.x); dx++)
    {
      const auto c = At({dx, dy});
      if (c != tc)
      {
        writer.Write(position + Vector2D<int>{dx, dy}, c);
      }
    }
  }
}

void Window::Write(Vector2D<int> pos, PixelColor c)
{
  data_[pos.y][pos.x] = c;
  shadow_buffer_.Writer().Write(pos, c);
}

const PixelColor &Window::At(Vector2D<int> pos) const
{
  return data_[pos.y][pos.x];
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

void Window::Move(Vector2D<int> dst_pos, const Rectangle<int> &src)
{
  shadow_buffer_.Move(dst_pos, src);
}