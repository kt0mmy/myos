#pragma once

#include <memory>
#include "graphics.hpp"
#include "window.hpp"

class Console
{
public:
    static const int kRows = 25, kColumns = 80;

    Console(const PixelColor &fg_color, const PixelColor &bg_color);
    void PutString(const char *s);
    void SetWriter(PixelWriter *writer);
    void SetWindow(const std::shared_ptr<Window>& window);

private:
    void Newline();
    void FillBackGround();
    void Refresh();
    
    PixelWriter *writer_;
    // writer で一つずつ書くのではなく、windowのコンソール領域を「ずらす」ことで効率化
    std::shared_ptr<Window> window_;
    const PixelColor fg_color_, bg_color_;
    char buffer_[kRows][kColumns + 1]; // 画面に表示している内容をバッファとして保持
    int cursor_row_, cursor_column_;
};