/*
 * @Author       : GameServer
 * @Date         : 2024-01-01
 * @Description  : 协程库基本使用示例
 */

#include "../include/coroutine.h"
#include "../include/sync.h"
#include <iostream>
#include <string>

using namespace coroutine;

// 简单的协程函数
void simple_coroutine(int id) {
    std::cout << "协程 " << id << " 开始执行" << std::endl;
    
    for (int i = 0; i < 3; ++i) {
        std::cout << "协程 " << id << " 执行步骤 " << i << std::endl;
        api::yield(); // 让出CPU
    }
    
    std::cout << "协程 " << id << " 执行完成" << std::endl;
}

// 生产者-消费者示例
Channel<int> g_channel(5); // 容量为5的通道

void producer(int id) {
    std::cout << "生产者 " << id << " 开始生产" << std::endl;
    
    for (int i = 0; i < 10; ++i) {
        int data = id * 100 + i;
        if (g_channel.send(data)) {
            std::cout << "生产者 " << id << " 生产了数据: " << data << std::endl;
        }
        api::yield();
    }
    
    std::cout << "生产者 " << id << " 生产完成" << std::endl;
}

void consumer(int id) {
    std::cout << "消费者 " << id << " 开始消费" << std::endl;
    
    for (int i = 0; i < 5; ++i) {
        int data;
        if (g_channel.receive(data)) {
            std::cout << "消费者 " << id << " 消费了数据: " << data << std::endl;
        }
        api::yield();
    }
    
    std::cout << "消费者 " << id << " 消费完成" << std::endl;
}

// 互斥锁示例
Mutex g_mutex;
int g_counter = 0;

void increment_counter(int id) {
    std::cout << "协程 " << id << " 开始增加计数器" << std::endl;
    
    for (int i = 0; i < 5; ++i) {
        {
            LockGuard lock(g_mutex);
            int old_value = g_counter;
            api::yield(); // 模拟处理时间
            g_counter = old_value + 1;
            std::cout << "协程 " << id << " 将计数器从 " << old_value << " 增加到 " << g_counter << std::endl;
        }
        api::yield();
    }
    
    std::cout << "协程 " << id << " 完成计数器增加" << std::endl;
}

// 信号量示例
Semaphore g_semaphore(2); // 最多允许2个协程同时访问

void access_resource(int id) {
    std::cout << "协程 " << id << " 尝试获取资源" << std::endl;
    
    g_semaphore.acquire();
    std::cout << "协程 " << id << " 获得资源访问权限" << std::endl;
    
    // 模拟使用资源
    for (int i = 0; i < 3; ++i) {
        std::cout << "协程 " << id << " 正在使用资源，步骤 " << i << std::endl;
        api::yield();
    }
    
    std::cout << "协程 " << id << " 释放资源" << std::endl;
    g_semaphore.release();
}

int main() {
    std::cout << "=== 协程库示例程序 ===" << std::endl;
    
    // 示例1: 基本协程调度
    std::cout << "\n--- 示例1: 基本协程调度 ---" << std::endl;
    for (int i = 1; i <= 3; ++i) {
        api::create([i]() { simple_coroutine(i); });
    }
    api::run();
    
    // 示例2: 生产者-消费者模式
    std::cout << "\n--- 示例2: 生产者-消费者模式 ---" << std::endl;
    api::create([](){ producer(1); });
    api::create([](){ producer(2); });
    api::create([](){ consumer(1); });
    api::create([](){ consumer(2); });
    api::create([](){ consumer(3); });
    api::create([](){ consumer(4); });
    api::run();
    
    // 关闭通道
    g_channel.close();
    
    // 示例3: 互斥锁
    std::cout << "\n--- 示例3: 互斥锁 ---" << std::endl;
    g_counter = 0;
    for (int i = 1; i <= 3; ++i) {
        api::create([i]() { increment_counter(i); });
    }
    api::run();
    std::cout << "最终计数器值: " << g_counter << std::endl;
    
    // 示例4: 信号量
    std::cout << "\n--- 示例4: 信号量 ---" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        api::create([i]() { access_resource(i); });
    }
    api::run();
    
    std::cout << "\n=== 所有示例执行完成 ===" << std::endl;
    return 0;
} 