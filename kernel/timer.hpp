#pragma once

#include <cstdint>
#include <queue>
#include <vector>
#include "message.hpp"

void InitializeLAPICTimer(std::deque<Message> &msg_queue);
void StartLAPICTimer();
uint32_t LAPICTimerElapsed();
void StopLAPICTimer();

void LAPICTimerOnInterrupt();

class Timer
{
public:
    Timer(unsigned long timeout, int value);
    unsigned long Timeout() const { return timeout_; }
    int Value() const { return value_; }

private:
    unsigned long timeout_;
    int value_; // タイムアウトに送信する値
};
// タイムアウトまで長いほど優先度低
inline bool operator<(const Timer& lhs, const Timer& rhs) {
    return lhs.Timeout() > rhs.Timeout();
};

class TimerManager
{
public:
    TimerManager(std::deque<Message> &msg_queue);
    void AddTimer(const Timer &timer);
    void Tick();
    unsigned long CurrentTick() const { return tick_; }

private:
    volatile unsigned long tick_{0};
    std::priority_queue<Timer> timers_{};
    std::deque<Message> &msg_queue_;
};

extern unsigned long lapic_timer_freq; // LAPICタイマーが1秒で何カウントするか
const int kTimerFreq = 100; // 1秒間にTimerManager::Freqが呼ばれる回数(10msごとに論理タイマがカウントされる)
extern TimerManager *timer_manager;
