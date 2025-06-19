#ifndef TIMER_H
#define TIMER_H

#include <functional>
#include <cstdint>
#include <vector>
#include <list>
#include <unordered_map>

namespace GameTimer {

using TimerCallback = std::function<void()>;
using TimerId = uint64_t;

class TimerWheel {
public:
    TimerWheel(uint32_t slotNum = 1024, uint32_t tickMs = 100);
    // 添加定时任务，delayMs为延迟毫秒数，返回任务ID
    TimerId addTimer(uint32_t delayMs, TimerCallback cb);
    // 取消定时任务
    void cancelTimer(TimerId id);
    // 推进时间轮，通常由主循环定时调用
    void tick();

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
};

} // namespace GameTimer

#endif // TIMER_H 