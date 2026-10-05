#pragma once
#include <vector>
#include <optional>
#include "graphics.hpp"
#include "shadow_buffer.hpp"
class Window
{
public:
    Window(int width, int height, PixelFormat shadow_format);
    ~Window() = default;
    // Windowインスタンスのコピーを禁止
    Window(const Window &rhs) = delete;
    Window &operator=(const Window &rhs) = delete;

    class WindowWriter : public PixelWriter
    {
    public:
        WindowWriter(Window &window) : window_{window} {}
        virtual void Write(Vector2D<int> pos, const PixelColor &c) override
        {
            window_.Write(pos, c);
        }
        virtual int Width() const override { return window_.Width(); }
        virtual int Height() const override { return window_.Height(); }

    private:
        Window &window_;
    };

    void DrawTo(FrameBuffer &screen, Vector2D<int> position);
    void DrawTo(FrameBuffer &screen, Vector2D<int> pos, const Rectangle<int> &area) const;
    void Write(Vector2D<int> pos, PixelColor c);

    void Move(Vector2D<int> dst_pos, const Rectangle<int> &src);
    const PixelColor &At(Vector2D<int> pos) const;

    int Width() const;
    int Height() const;
    Vector2D<int> Size() const;

    WindowWriter *Writer();

    void SetTranparentColor(std::optional<PixelColor> c);

private:
    int width_, height_;
    std::vector<std::vector<PixelColor>> data_{};
    WindowWriter writer_{*this};
    std::optional<PixelColor> transparent_color_{std::nullopt};

    FrameBuffer shadow_buffer_{};
};