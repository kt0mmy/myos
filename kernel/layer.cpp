#include "layer.hpp"
#include <algorithm>

Layer::Layer(unsigned int id) : id_{id} {}

unsigned int Layer::ID() const
{
    return id_;
}

std::shared_ptr<Window> Layer::GetWindow() const
{
    return window_;
}

Layer &Layer::SetWindow(const std::shared_ptr<Window> &window)
{
    window_ = window;
    return *this;
}

// 再描画はせず、レイヤーの位置情報を更新するだけ
Layer &Layer::Move(Vector2D<int> pos)
{
    pos_ = pos;
    return *this;
}

Layer &Layer::MoveRelative(Vector2D<int> pos_diff)
{
    pos_ += pos_diff;
    return *this;
}

void Layer::DrawTo(PixelWriter &writer) const
{
    if (window_)
        window_->DrawTo(writer, pos_);
}

Layer &LayerManager::NewLayer()
{
    latest_id_++;
    return *layers_.emplace_back(new Layer(latest_id_));
}

void LayerManager::SetWriter(PixelWriter *writer)
{
    writer_ = writer;
}

void LayerManager::Draw() const
{
    for (auto layer : layer_stack_)
    {
        layer->DrawTo(*writer_);
    }
}

void LayerManager::Move(unsigned int id, Vector2D<int> new_position)
{
    FindLayer(id)->Move(new_position);
}

void LayerManager::MoveRelative(unsigned int id, Vector2D<int> pos_diff)
{
    FindLayer(id)->MoveRelative(pos_diff);
}

/**
 * @param id
 * @param new_height  layer_stack_ の中での位置
 *
 * ex. (A, 2) にする
 * [A][B][C][D]
 *  0  1  2  3
 *
 * erase(0)
 * [B][C][D]
 *  0  1  2  3
 *
 * insert(2)
 * [B][C][A][D]
 *  0  1  2  3
 *
 * ex. Aを指定、new_heightを4以上にした場合
 * erase
 * [B][C][D]
 *  0  1  2  
 *
 * insert(4 - 1 == end)
 * [B][C][D][A]
 *  0  1  2  3
 *
 * ex. Aを指定、new_heightを3にした場合
 * erase(0)
 * [B][C][D]
 *  0  1  2  3
 *
 * insert(3 == end)
 * [B][C][D][A]
 *  0  1  2  3
 *
 *
 * */
void LayerManager::UpDown(unsigned int id, int new_height)
{
    if (new_height < 0)
    {
        Hide(id);
        return;
    }
    new_height = std::min(static_cast<size_t>(new_height), layer_stack_.size());

    auto layer = FindLayer(id);
    auto old_pos = std::find(layer_stack_.begin(), layer_stack_.end(), layer);
    auto new_pos = layer_stack_.begin() + new_height;

    // 非表示レイヤーの場合
    if (old_pos == layer_stack_.end())
    {
        layer_stack_.insert(new_pos, layer);
        return;
    }

    if (new_pos == layer_stack_.end()) {
        new_height--;
    }

    layer_stack_.erase(old_pos);
    new_pos = layer_stack_.begin() + new_height;
    layer_stack_.insert(new_pos, layer);
}

void LayerManager::Hide(unsigned int id)
{
    auto layer = FindLayer(id);
    auto pos = std::find(layer_stack_.begin(), layer_stack_.end(), layer);
    if (pos != layer_stack_.end())
    {
        layer_stack_.erase(pos);
    }
}

Layer *LayerManager::FindLayer(unsigned int id)
{
    auto pred = [id](const std::unique_ptr<Layer> &elem)
    {
        return elem->ID() == id;
    };
    auto it = std::find_if(layers_.begin(), layers_.end(), pred);
    if (it == layers_.end())
        return nullptr;
    return it->get();
}


LayerManager* layer_manager;