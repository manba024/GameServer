#include "timer.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    auto timer = GameTimer::createTimer(GameTimer::TimerType::Wheel);
    timer->run();
    timer->addTimer(500, [](){ std::cout << "500ms timer!\n"; });
    timer->addTimer(1500, [](){ std::cout << "1500ms timer!\n"; });
    std::this_thread::sleep_for(std::chrono::seconds(3));
    timer->stop();
    return 0;
} 