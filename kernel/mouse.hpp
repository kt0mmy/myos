#pragma once

#include "graphics.hpp"

const int kMouseCursorWidth = 15;
const int kMouseCursorHeight = 24;

class MouseCursor
{
public:
    MouseCursor(PixelWriter *writer, PixelColor erase_color, Vector2D<int> init_position);
    void MoveRelative(Vector2D<int> displacement);

private:
    PixelWriter *pixel_writer_ = nullptr;
    PixelColor erase_color_;
    Vector2D<int> position_;
};