#include "font.hpp"

// objcopy で生成したオブジェクトファイルで定義されているシンボル
extern const uint8_t _binary_hankaku_bin_start; // データの開始仮想アドレス(データを格納する変数ではなく、データの先頭位置を示すリンカシンボル)
extern const uint8_t _binary_hankaku_bin_end;   // 終了アドレス + 1
extern const uint8_t _binary_hankaku_bin_size;  // データのトータルサイズ(バイト)

const uint8_t *GetFont(char c)
{
    auto index = 16 * static_cast<unsigned int>(c); // 1 文字は16バイト(1行は8ビット)
    if (index >= reinterpret_cast<uintptr_t>(&_binary_hankaku_bin_size))
    {
        return nullptr;
    }
    return &_binary_hankaku_bin_start + index;
}

void WriteAscii(PixelWriter &writer, Vector2D<int> pos, char c, const PixelColor &color)
{
    const uint8_t *font = GetFont(c);
    if (font == nullptr)
        return;

    for (int dy = 0; dy < 16; dy++)
    {
        for (int dx = 0; dx < 8; dx++)
        {
            if ((font[dy] << dx) & 0x80u)
            {
                writer.Write(pos + Vector2D<int>{dx, dy}, color);
            }
        }
    }
}

void WriteString(PixelWriter &writer, Vector2D<int> pos, const char *s, const PixelColor &color)
{
    Vector2D<int> cursor = {pos.x, pos.y};

    for (int i = 0; s[i] != '\0'; i++)
    {
        if (s[i] == '\n')
        {
            cursor += Vector2D<int>{0, 16};
        }
        else
        {
            cursor += Vector2D<int>{8, 0};
            WriteAscii(writer, cursor, s[i], color);
        }
    }
}