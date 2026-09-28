#include "timer.hpp"

namespace
{
    const uint32_t kCountMax = 0xffffffffu;
    // Local API タイマのレジスタ
    // レジスタの値に対して参照を取るので、代入 == レジスタへの書き込み
    volatile uint32_t &lvt_timer = *reinterpret_cast<uint32_t *>(0xfee00320);
    volatile uint32_t &initial_count = *reinterpret_cast<uint32_t *>(0xfee00380);
    volatile uint32_t &current_count = *reinterpret_cast<uint32_t *>(0xfee00390);
    volatile uint32_t &divide_config = *reinterpret_cast<uint32_t *>(0xfee003e0); // カウンタの減り度合いを制御
}

void InitializeLAPICTimer()
{
    divide_config = 0b1011;
    lvt_timer = (0b001 << 16) | 32; // 単発、割り込みを発生させない
}

void StartLAPICTimer()
{
    initial_count = kCountMax;
}

uint32_t LAPICTimerElapsed()
{
    return kCountMax - current_count;
}

void StopLAPICTimer()
{
    initial_count = 0;
}