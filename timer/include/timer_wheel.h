#ifndef TIMER_WHEEL_H
#define TIMER_WHEEL_H

#include "timer.h"
#include <vector>
#include <list>
#include <unordered_map>
#include <thread>
#include <atomic>

namespace GameTimer {

class TimerWheel : public ITimer {
public:
    TimerWheel(uint32_t slotNum = 1024, uint32_t tickMs = 100);
    ~TimerWheel();
    TimerId addTimer(uint32_t delayMs, TimerCallback cb) override;
    void cancelTimer(TimerId id) override;
    void tick() override;
    void run() override;
    void stop() override;
private:
    struct TimerTask {
        TimerId id;
        uint32_t rotations;
        TimerCallback cb;
    };
    std::vector<std::list<TimerTask>> slots_;
    std::unordered_map<TimerId, std::pair<uint32_t, std::list<TimerTask>::iterator>> idMap_;
    uint32_t slotNum_;
    uint32_t tickMs_;
    uint32_t curSlot_;
    TimerId nextId_;
    std::thread tickThread_;
    std::atomic<bool> running_;
};

} // namespace GameTimer

#endif // TIMER_WHEEL_H 