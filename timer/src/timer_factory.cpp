#include "timer.h"
#include "timer_wheel.h"
#include <memory>

namespace GameTimer {

std::unique_ptr<ITimer> createTimer(TimerType type) {
    switch (type) {
        case TimerType::Wheel:
            return std::unique_ptr<ITimer>(new TimerWheel());
        // case TimerType::Heap:
        //     return std::unique_ptr<ITimer>(new TimerHeap());
        // case TimerType::RBTree:
        //     return std::unique_ptr<ITimer>(new TimerRBTree());
        // case TimerType::List:
        //     return std::unique_ptr<ITimer>(new TimerList());
        default:
            return nullptr;
    }
}

} // namespace GameTimer 