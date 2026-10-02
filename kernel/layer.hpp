#pragma once
#include <memory>
#include <map>
#include <vector>
#include "graphics.hpp"
#include "window.hpp"

class Layer
{
public:
    Layer(unsigned int id = 0);

    unsigned int ID() const;
    std::shared_ptr<Window> GetWindow() const;

    Layer &SetWindow(const std::shared_ptr<Window> &window);

    // 再描画はせず、レイヤーの位置情報を更新するだけ
    Layer &Move(Vector2D<int> pos);
    Layer &MoveRelative(Vector2D<int> pos_diff);

    void DrawTo(FrameBuffer &screen) const;

private:
    unsigned int id_;
    Vector2D<int> pos_;
    std::shared_ptr<Window> window_;
};

class LayerManager
{

public:
    Layer &NewLayer();
    void SetWriter(FrameBuffer *screen);
    void Draw() const;
    void Move(unsigned int id, Vector2D<int> new_position);
    void MoveRelative(unsigned int id, Vector2D<int> pos_diff);
    void UpDown(unsigned int id, int new_height);
    void Hide(unsigned int id);

private:
    // nullptr なので、直接メモリに書いていく
    FrameBuffer *screen_{nullptr};
    // NOTE: 全レイヤーを保持
    std::vector<std::unique_ptr<Layer>> layers_{};
    // NOTE: layers_の中から、表示対象のレイヤーのみ抜き出して管理
    std::vector<Layer *> layer_stack_{};
    unsigned int latest_id_{0};

    Layer *FindLayer(unsigned int id);
};

extern LayerManager *layer_manager;