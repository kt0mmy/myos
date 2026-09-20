#include "interrupt.hpp"


void NotifyEndOfInterrupt() {
    // コンパイラの最適化対象外とする
    // 書き込んだ値を読み込む処理がないため、書き込み命令が省かれる可能性がある
    volatile auto end_of_interrupt = reinterpret_cast<uint32_t*>(0xfee000b0);
    *end_of_interrupt = 0;
}