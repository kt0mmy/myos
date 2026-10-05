#include "shadow_buffer.hpp"

namespace
{

  int BitsPerPixel(PixelFormat pixel_format)
  {
    switch (pixel_format)
    {
    case kPixelBGRResv8BitPerColor:
      return 32;
    case kPixelRGBResv8BitPerColor:
      return 32;
    default:
      return -1;
    }
  }
  int BytesPerPixel(PixelFormat pixel_format)
  {
    const auto bits_per_pixel = BitsPerPixel(pixel_format);
    if (bits_per_pixel < 0)
      return bits_per_pixel;
    else
      return (bits_per_pixel + 7) / 8;
  }

  int BytesPerScanLine(const FrameBuferConfig &config)
  {
    return config.pixels_per_scan_line * BytesPerPixel(config.pixel_format);
  }

  uint8_t *FrameAddrAt(Vector2D<int> pos, const FrameBuferConfig &config)
  {

    return config.frame_buffer + BytesPerScanLine(config) * pos.y + BytesPerPixel(config.pixel_format) * pos.x;
  }

  Vector2D<int> FrameBufferSize(const FrameBuferConfig &config)
  {
    return {static_cast<int>(config.horizontal_resolution), static_cast<int>(config.vertical_resolution)};
  }
}

Error FrameBuffer::Initialize(const FrameBuferConfig &config)
{
  config_ = config;

  const auto bytes_per_pixel = BytesPerPixel(config_.pixel_format);
  if (bytes_per_pixel <= 0)
  {
    return MAKE_ERROR(Error::kUnknownPixelFormat);
  }

  if (config_.frame_buffer)
  {
    buffer_.resize(0);
  }
  else
  {
    const auto bytes = bytes_per_pixel * config_.horizontal_resolution * config_.vertical_resolution;
    buffer_.resize(bytes);
    config_.frame_buffer = buffer_.data();
    config_.pixels_per_scan_line = config_.horizontal_resolution;
  }

  switch (config_.pixel_format)
  {
  case kPixelBGRResv8BitPerColor:
    writer_ = std::make_unique<BGRResv8BitPerColorPixelWriter>(config_);
    break;
  case kPixelRGBResv8BitPerColor:
    writer_ = std::make_unique<RGBResv8BitPerColorPixelWriter>(config_);
    break;
  default:
    return MAKE_ERROR(Error::kUnknownPixelFormat);
  }

  return MAKE_ERROR(Error::kSuccess);
}

Error FrameBuffer::Copy(Vector2D<int> pos, const FrameBuffer &src)
{
  if (config_.pixel_format != src.config_.pixel_format)
  {
    return MAKE_ERROR(Error::kUnknownPixelFormat);
  }

  const auto bytes_per_pixel = BytesPerPixel(config_.pixel_format);
  if (bytes_per_pixel <= 0)
  {
    return MAKE_ERROR(Error::kUnknownPixelFormat);
  }

  const auto dst_size = FrameBufferSize(config_);
  const auto src_size = FrameBufferSize(src.config_);
  const Vector2D<int> dst_start = ElementMax(pos, {0, 0});
  const Vector2D<int> dst_end = ElementMin(pos + src_size, dst_size);

  uint8_t *dst_buf = FrameAddrAt(dst_start, config_);
  const uint8_t *src_buf = FrameAddrAt({0, 0}, src.config_);

  for (int dy = dst_start.y; dy < dst_end.y; dy++)
  {
    memcpy(dst_buf, src_buf, bytes_per_pixel * (dst_end.x - dst_start.x));
    dst_buf += BytesPerScanLine(config_);
    src_buf += BytesPerScanLine(src.config_);
  }

  return MAKE_ERROR(Error::kSuccess);
}

/**
 * @param pos フレームバッファ座標系における、描画領域の指定位置
 * @param src
 * @param src_area src の座標系における描画領域
 */
Error FrameBuffer::Copy(Vector2D<int> pos, const FrameBuffer &src, const Rectangle<int> &src_area)
{
  if (config_.pixel_format != src.config_.pixel_format)
  {
    return MAKE_ERROR(Error::kUnknownPixelFormat);
  }

  const auto bytes_per_pixel = BytesPerPixel(config_.pixel_format);
  if (bytes_per_pixel <= 0)
  {
    return MAKE_ERROR(Error::kUnknownPixelFormat);
  }

  const Rectangle<int> src_area_shifted{pos, src_area.size};
  const Rectangle<int> src_outline{pos - src_area.pos, FrameBufferSize(src.config_)}; // pos 基準のsrc Rectangle
  const Rectangle<int> dst_outline{{0, 0}, FrameBufferSize(config_)};

  const auto copy_area = dst_outline & src_outline & src_area_shifted;
  const auto src_start_pos = copy_area.pos - (pos - src_area.pos);

  uint8_t *dst_buf = FrameAddrAt(copy_area.pos, config_);
  const uint8_t *src_buf = FrameAddrAt(src_start_pos, src.config_);

  for (int y = 0; y < copy_area.size.y; y++)
  {
    memcpy(dst_buf, src_buf, bytes_per_pixel * copy_area.size.x);
    dst_buf += BytesPerScanLine(config_);
    src_buf += BytesPerScanLine(src.config_);
  }

  return MAKE_ERROR(Error::kSuccess);
}

void FrameBuffer::Move(Vector2D<int> dst_pos, const Rectangle<int> &src)
{
  const auto bytes_per_pixel = BytesPerPixel(config_.pixel_format);
  const auto bytes_per_scan_line = BytesPerScanLine(config_);

  // 上に移動させる
  if (dst_pos.y < src.pos.y)
  {
    uint8_t *dst_buf = FrameAddrAt(dst_pos, config_);
    const uint8_t *src_buf = FrameAddrAt(src.pos, config_);
    for (int y = 0; y < src.size.y; y++)
    {
      memcpy(dst_buf, src_buf, src.size.x * bytes_per_pixel);
      dst_buf += bytes_per_scan_line;
      src_buf += bytes_per_scan_line;
    }
  }
  else
  {
    uint8_t *dst_buf = FrameAddrAt(dst_pos + Vector2D<int>{0, src.size.y - 1}, config_);
    const uint8_t *src_buf = FrameAddrAt(src.pos + Vector2D<int>{0, src.size.y - 1}, config_);
    for (int y = 0; y < src.size.y; y++)
    {
      memcpy(dst_buf, src_buf, src.size.x * bytes_per_pixel);
      dst_buf -= bytes_per_scan_line;
      src_buf -= bytes_per_scan_line;
    }
  }
}
const FrameBuferConfig FrameBuffer::Config() const
{
  return config_;
}