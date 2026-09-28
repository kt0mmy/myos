#pragma once
#include <vector>
#include <optional>
#include "graphics.hpp"

class Window
{
public:
    Window(int width, int height);
    ~Window() = default;
    // Windowインスタンスのコピーを禁止
    Window(const Window &rhs) = delete;
    Window &operator=(const Window &rhs) = delete;

    class WindowWriter : public PixelWriter
    {
    public:
        WindowWriter(Window &window) : window_{window} {}
        virtual void Write(int x, int y, const PixelColor &c) override
        {
            window_.At(x, y) = c;
        }
        virtual int Width() const override { return window_.Width(); }
        virtual int Height() const override { return window_.Height(); }

    private:
        Window &window_;
    };

    void DrawTo(PixelWriter &writer, Vector2D<int> position);
    PixelColor &At(int x, int y);
    const PixelColor &At(int x, int y) const;

    int Width() const;
    int Height() const;

    WindowWriter *Writer();

    void SetTranparentColor(std::optional<PixelColor> c);

private:
    int width_, height_;
    std::vector<std::vector<PixelColor>> data_{};
    WindowWriter writer_{*this};
    std::optional<PixelColor> transparent_color_{std::nullopt};
};