#include "interrupt.hpp"
#include "asmfunc.h"
#include "segment.hpp"

// NOTE: 割り込みを発生させる側の仕組み

std::array<InterruptDescriptor, 256> idt;

namespace
{
    std::deque<Message> *msg_queue;

    __attribute__((interrupt)) void IntHandlerXHCI(InterruptFrame *frame)
    {
        msg_queue->push_back(Message{Message::kInterruptXHCI});
        NotifyEndOfInterrupt();
    }
} // namespace

void NotifyEndOfInterrupt()
{
    // コンパイラの最適化対象外とする
    // 書き込んだ値を読み込む処理がないため、書き込み命令が省かれる可能性がある
    volatile auto end_of_interrupt = reinterpret_cast<uint32_t *>(0xfee000b0);
    *end_of_interrupt = 0;
}

void SetIDTEntry(InterruptDescriptor &desc, InterruptDescriptorAttribute attr, uint64_t offset, uint16_t segment_selector)
{
    desc.attr = attr;
    desc.offset_low = offset & 0xffff;
    desc.offset_middle = (offset >> 16) & 0xffff;
    desc.offset_high = offset >> 32;
    desc.segment_selector = segment_selector;
}

void InitializeInterrupt(std::deque<Message> *msg_queue)
{

    ::msg_queue = msg_queue;
    SetIDTEntry(idt[InterruptVector::kXHCI], MakeIDTAttr(DescriptorType::kInterruptGate, 0), reinterpret_cast<uint64_t>(IntHandlerXHCI), kKernelCS);
    LoadIDT(sizeof(idt) - 1, reinterpret_cast<uintptr_t>(&idt[0]));
}