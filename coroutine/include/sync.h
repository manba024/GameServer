/*
 * @Author       : GameServer
 * @Date         : 2024-01-01
 * @Description  : 协程同步原语
 */

#ifndef COROUTINE_SYNC_H
#define COROUTINE_SYNC_H

#include "coroutine.h"
#include <queue>
#include <chrono>

namespace coroutine {

/**
 * @brief 协程互斥锁
 * 
 * 提供协程间的互斥访问控制
 */
class Mutex {
public:
    Mutex() : locked_(false) {}
    
    /**
     * @brief 加锁
     */
    void lock();
    
    /**
     * @brief 尝试加锁
     * @return 是否成功获取锁
     */
    bool try_lock();
    
    /**
     * @brief 解锁
     */
    void unlock();
    
    /**
     * @brief 检查是否已锁定
     */
    bool is_locked() const { return locked_; }
    
private:
    bool locked_;                           // 锁状态
    std::queue<CoroutineId> waiting_queue_; // 等待队列
};

/**
 * @brief 协程条件变量
 * 
 * 提供协程间的条件同步机制
 */
class ConditionVariable {
public:
    ConditionVariable() = default;
    
    /**
     * @brief 等待条件
     * @param mutex 关联的互斥锁
     */
    void wait(Mutex& mutex);
    
    /**
     * @brief 超时等待条件
     * @param mutex 关联的互斥锁
     * @param timeout_ms 超时时间（毫秒）
     * @return 是否在超时前被唤醒
     */
    bool wait_for(Mutex& mutex, int timeout_ms);
    
    /**
     * @brief 唤醒一个等待的协程
     */
    void notify_one();
    
    /**
     * @brief 唤醒所有等待的协程
     */
    void notify_all();
    
private:
    std::queue<CoroutineId> waiting_queue_; // 等待队列
};

/**
 * @brief 协程信号量
 * 
 * 提供计数信号量功能
 */
class Semaphore {
public:
    /**
     * @brief 构造函数
     * @param count 初始计数值
     */
    explicit Semaphore(int count = 0) : count_(count) {}
    
    /**
     * @brief 获取信号量（P操作）
     */
    void acquire();
    
    /**
     * @brief 尝试获取信号量
     * @return 是否成功获取
     */
    bool try_acquire();
    
    /**
     * @brief 释放信号量（V操作）
     */
    void release();
    
    /**
     * @brief 获取当前计数值
     */
    int count() const { return count_; }
    
private:
    int count_;                             // 计数值
    std::queue<CoroutineId> waiting_queue_; // 等待队列
};

/**
 * @brief 协程通道
 * 
 * 提供协程间的消息传递机制
 */
template<typename T>
class Channel {
public:
    /**
     * @brief 构造函数
     * @param capacity 通道容量（0表示无缓冲）
     */
    explicit Channel(size_t capacity = 0) : capacity_(capacity), closed_(false) {}
    
    /**
     * @brief 发送数据
     * @param data 要发送的数据
     * @return 是否成功发送
     */
    bool send(const T& data);
    
    /**
     * @brief 接收数据
     * @param data 接收到的数据
     * @return 是否成功接收
     */
    bool receive(T& data);
    
    /**
     * @brief 尝试发送数据（非阻塞）
     * @param data 要发送的数据
     * @return 是否成功发送
     */
    bool try_send(const T& data);
    
    /**
     * @brief 尝试接收数据（非阻塞）
     * @param data 接收到的数据
     * @return 是否成功接收
     */
    bool try_receive(T& data);
    
    /**
     * @brief 关闭通道
     */
    void close();
    
    /**
     * @brief 检查通道是否已关闭
     */
    bool is_closed() const { return closed_; }
    
    /**
     * @brief 获取通道中的数据数量
     */
    size_t size() const { return buffer_.size(); }
    
    /**
     * @brief 检查通道是否为空
     */
    bool empty() const { return buffer_.empty(); }
    
    /**
     * @brief 检查通道是否已满
     */
    bool full() const { return capacity_ > 0 && buffer_.size() >= capacity_; }
    
private:
    size_t capacity_;                       // 通道容量
    std::queue<T> buffer_;                  // 数据缓冲区
    bool closed_;                           // 关闭标志
    std::queue<CoroutineId> send_queue_;    // 发送等待队列
    std::queue<CoroutineId> recv_queue_;    // 接收等待队列
};

/**
 * @brief 协程锁守卫
 * 
 * RAII风格的锁管理
 */
class LockGuard {
public:
    explicit LockGuard(Mutex& mutex) : mutex_(mutex) {
        mutex_.lock();
    }
    
    ~LockGuard() {
        mutex_.unlock();
    }
    
private:
    Mutex& mutex_;
    
    // 禁用拷贝构造和赋值
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
};

} // namespace coroutine

#endif // COROUTINE_SYNC_H 