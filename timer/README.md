# GameTimer 定时器库

适用于游戏服务器的高性能定时器库，采用单层时间轮（Time Wheel）实现，支持高频率、大量定时任务的添加与管理，适合 buff、技能、怪物刷新、活动倒计时等场景。

## 原理简介
- 时间轮将时间划分为若干槽（slot），每个槽对应一个时间片（tick）。
- 定时任务根据到期时间被分配到对应槽，tick 推进时检查当前槽到期任务。
- 插入、删除、触发任务效率高，适合大规模定时场景。

## 主要接口
- `TimerWheel(uint32_t slotNum = 1024, uint32_t tickMs = 100)`  构造函数，slotNum 槽数量，tickMs 每个槽的毫秒数。
- `TimerId addTimer(uint32_t delayMs, TimerCallback cb)`  添加定时任务，delayMs 为延迟毫秒数，cb 为回调。
- `void cancelTimer(TimerId id)`  取消定时任务。
- `void tick()`  推进时间轮，通常由主循环定时调用。

## 用法示例
```cpp
#include "timer.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    GameTimer::TimerWheel timer;
    timer.addTimer(500, [](){ std::cout << "500ms timer!\n"; });
    timer.addTimer(1500, [](){ std::cout << "1500ms timer!\n"; });
    for (int i = 0; i < 20; ++i) {
        timer.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
```

## 适用场景
- 大量 buff/技能过期管理
- 怪物/副本定时生成与销毁
- 活动倒计时、心跳检测等

## 扩展建议
- 可升级为分层时间轮以支持更大时间跨度
- 可加锁支持多线程
- 支持定时任务重复执行 

## 补充说明（来自 deepseek）

### 💎 技术选型建议

- **高精度需求**（如战斗技能CD）：优先选择**时间轮**（Time Wheel）或**优先队列**（PriorityQueue）。
- **复杂调度**（如跨服活动）：可使用**Quartz**等框架，支持 Crontab 表达式。
- **轻量级任务**（如心跳检测）：可采用 **ScheduledThreadPoolExecutor**（Java）或协程（Unity）。

> ⚠️ **注意事项**  
> - 定时器实现需考虑**线程安全**（如加锁机制）、**时间精度**（毫秒级或秒级）、以及**系统时间修改**的兼容性（如避免因系统时间调整导致任务紊乱）。
> - 通过合理设计定时器系统，游戏服务器可高效支撑从底层运维到上层玩法的全链路需求，确保游戏流畅性与逻辑准确性。🚀