#ifndef TIMER_H
#define TIMER_H

#include <functional>
#include <cstdint>
#include <memory>

namespace GameTimer {

using TimerCallback = std::function<void()>;
using TimerId = uint64_t;

enum class TimerType {
    Wheel,
    Heap,
    RBTree,
    List
};

class ITimer {
public:
    virtual ~ITimer() = default;
    virtual TimerId addTimer(uint32_t delayMs, TimerCallback cb) = 0;
    virtual void cancelTimer(TimerId id) = 0;
    virtual void tick() = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
};

// 工厂方法，根据类型创建定时器实例
std::unique_ptr<ITimer> createTimer(TimerType type);

} // namespace GameTimer

#endif // TIMER_H 