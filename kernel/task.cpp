#include "task.hpp"
#include "timer.hpp"
#include "asmfunc.h"

// fxsave_areaはFXSAVE命令の都合、16バイトでアライメントが必要
// そこで、コンテキストを16バイトの倍数に配置しておくと、今のコンテキストの作り上
// 16バイトアライメントで配置される
alignas(16) TaskContext task_b_ctx, task_a_ctx;

namespace
{
    TaskContext *current_task;
}

void SwitchTask() {
    TaskContext* old_task = current_task;
    if (current_task == &task_a_ctx) {
        current_task = &task_b_ctx;
    } else {
        current_task = &task_a_ctx;
    }
    SwitchContext(current_task, old_task);
}

void InitializeTask()
{
    current_task = &task_a_ctx;

    __asm__("cli");
    timer_manager->AddTimer(
        Timer{timer_manager->CurrentTick() + kTaskTimerPeriod, kTaskTimerValue});
    __asm__("sti");
}