#include "task.hpp"
#include "timer.hpp"
#include "asmfunc.h"
#include "segment.hpp"

// fxsave_areaはFXSAVE命令の都合、16バイトでアライメントが必要
// そこで、コンテキストを16バイトの倍数に配置しておくと、今のコンテキストの作り上
// 16バイトアライメントで配置される
alignas(16) TaskContext task_b_ctx, task_a_ctx;

namespace
{
    TaskContext *current_task;
}


TaskManager* task_manager;

void InitializeTask()
{
    task_manager = new TaskManager;

    __asm__("cli");
    timer_manager->AddTimer(
        Timer{timer_manager->CurrentTick() + kTaskTimerPeriod, kTaskTimerValue});
    __asm__("sti");
}

Task::Task(uint64_t id) : id_{id} {}

Task &Task::InitContext(TaskFunc *f, int64_t data)
{
    const size_t stack_size = kDefaultSizeStackBytes / sizeof(stack_[0]);
    stack_.resize(stack_size);
    uint64_t stack_end = reinterpret_cast<uint64_t>(&stack_[stack_size]);

    memset(&context_, 0, sizeof(context_));

    context_.rip = reinterpret_cast<uint64_t>(f);
    context_.rdi = id_;  // 第1引数
    context_.rsi = data; // 第2引数

    context_.cr3 = GetCR3();
    context_.rflags = 0x202; // 割り込み許可
    context_.cs = kKernelCS;
    context_.ss = kKernelSS;
    // call命令実行時点で、rspは16の倍数である必要がある
    // call命令実行後、戻り先アドレスを積むので下一桁は必ず8になる
    // ↑これを前提に、コンパイラは変数を配置する
    // TaskBに切り替えたとき、あたかも関数呼び出し後に見せかけるため、
    // 16の倍数にした上で8を引いている
    context_.rsp = (stack_end & ~0xflu) - 8;

    // MXCSRレジスタはすべて1にしないと、浮動小数点計算が動かない
    *reinterpret_cast<uint32_t *>(&task_b_ctx.fxsave_area[24]) = 0x1f80;

    return *this;
}

TaskContext &Task::Context()
{
    return context_;
}

TaskManager::TaskManager()
{
    // 現在実行中のコンテキストに対応させる
    NewTask();
}

Task &TaskManager::NewTask()
{
    latest_id_++;
    return *tasks_.emplace_back(new Task{latest_id_});
}
void TaskManager::SwitchTask()
{
    size_t next_task_index = (current_task_index_ + 1) % tasks_.size();
    Task &current_task = *tasks_[current_task_index_];
    Task &next_task = *tasks_[next_task_index];
    current_task_index_ = next_task_index;
    SwitchContext(&next_task.Context(), &current_task.Context());
}