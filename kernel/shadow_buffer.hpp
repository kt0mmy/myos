#pragma once

#include <vector>
#include <memory>

#include "graphics.hpp"
#include "error.hpp"
#include "frame_buffer_config.hpp"

// フレームバッファやそれと同等のシャドウバッファを表す
// フレームバッファの場合、buffer は使わず、frame
class FrameBuffer 
{
public:
    Error Initialize(const FrameBuferConfig &config);
    Error Copy(Vector2D<int> pos, const FrameBuffer &src);
    void Move(Vector2D<int> pos, const Rectangle<int>& src);

    FrameBufferWriter &Writer() { return *writer_; }

private:
    FrameBuferConfig config_{};
    std::vector<uint8_t> buffer_{};
    std::unique_ptr<FrameBufferWriter> writer_{};
};