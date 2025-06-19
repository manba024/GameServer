#include "timer.h"
#include <iostream>

namespace GameTimer {

TimerWheel::TimerWheel(uint32_t slotNum, uint32_t tickMs)
    : slotNum_(slotNum), tickMs_(tickMs), curSlot_(0), nextId_(1) {
    slots_.resize(slotNum_);
}

TimerId TimerWheel::addTimer(uint32_t delayMs, TimerCallback cb) {
    if (delayMs == 0) delayMs = tickMs_;
    uint32_t ticks = delayMs / tickMs_;
    uint32_t rotations = ticks / slotNum_;
    uint32_t slot = (curSlot_ + (ticks % slotNum_)) % slotNum_;
    TimerTask task{nextId_++, rotations, std::move(cb)};
    slots_[slot].push_back(task);
    auto it = --slots_[slot].end();
    idMap_[task.id] = {slot, it};
    return task.id;
}

void TimerWheel::cancelTimer(TimerId id) {
    auto it = idMap_.find(id);
    if (it != idMap_.end()) {
        uint32_t slot = it->second.first;
        slots_[slot].erase(it->second.second);
        idMap_.erase(it);
    }
}

void TimerWheel::tick() {
    auto &slotList = slots_[curSlot_];
    for (auto it = slotList.begin(); it != slotList.end(); ) {
        if (it->rotations == 0) {
            // 到期，执行回调
            it->cb();
            idMap_.erase(it->id);
            it = slotList.erase(it);
        } else {
            --(it->rotations);
            ++it;
        }
    }
    curSlot_ = (curSlot_ + 1) % slotNum_;
}

} // namespace GameTimer 