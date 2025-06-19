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