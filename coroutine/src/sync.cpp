/*
 * @Author       : GameServer
 * @Date         : 2024-01-01
 * @Description  : 协程同步原语实现
 */

#include "../include/sync.h"
#include <iostream>

namespace coroutine {

//=============================================================================
// Mutex Implementation
//=============================================================================

void Mutex::lock() {
    if (!locked_) {
        locked_ = true;
        return;
    }
    
    // 如果已经被锁定，将当前协程加入等待队列
    CoroutineId current_id = Scheduler::getInstance().getCurrentCoroutineId();
    if (current_id != 0) {
        waiting_queue_.push(current_id);
        Scheduler::getInstance().yield();
    }
}

bool Mutex::try_lock() {
    if (!locked_) {
        locked_ = true;
        return true;
    }
    return false;
}

void Mutex::unlock() {
    if (!locked_) {
        return;
    }
    
    locked_ = false;
    
    // 唤醒等待队列中的一个协程
    if (!waiting_queue_.empty()) {
        CoroutineId next_id = waiting_queue_.front();
        waiting_queue_.pop();
        locked_ = true; // 立即给下一个协程加锁
        Scheduler::getInstance().resume(next_id);
    }
}

//=============================================================================
// ConditionVariable Implementation
//=============================================================================

void ConditionVariable::wait(Mutex& mutex) {
    CoroutineId current_id = Scheduler::getInstance().getCurrentCoroutineId();
    if (current_id == 0) {
        return; // 主协程不能等待
    }
    
    // 将当前协程加入等待队列
    waiting_queue_.push(current_id);
    
    // 释放互斥锁
    mutex.unlock();
    
    // 挂起当前协程
    Scheduler::getInstance().yield();
    
    // 被唤醒后重新获取锁
    mutex.lock();
}

bool ConditionVariable::wait_for(Mutex& mutex, int timeout_ms) {
    // 简化实现，实际需要定时器支持
    (void)timeout_ms; // 抑制未使用参数警告
    wait(mutex);
    return true; // 假设总是成功
}

void ConditionVariable::notify_one() {
    if (!waiting_queue_.empty()) {
        CoroutineId next_id = waiting_queue_.front();
        waiting_queue_.pop();
        Scheduler::getInstance().resume(next_id);
    }
}

void ConditionVariable::notify_all() {
    while (!waiting_queue_.empty()) {
        CoroutineId next_id = waiting_queue_.front();
        waiting_queue_.pop();
        Scheduler::getInstance().resume(next_id);
    }
}

//=============================================================================
// Semaphore Implementation
//=============================================================================

void Semaphore::acquire() {
    if (count_ > 0) {
        count_--;
        return;
    }
    
    // 如果计数为0，将当前协程加入等待队列
    CoroutineId current_id = Scheduler::getInstance().getCurrentCoroutineId();
    if (current_id != 0) {
        waiting_queue_.push(current_id);
        Scheduler::getInstance().yield();
    }
}

bool Semaphore::try_acquire() {
    if (count_ > 0) {
        count_--;
        return true;
    }
    return false;
}

void Semaphore::release() {
    count_++;
    
    // 唤醒等待队列中的一个协程
    if (!waiting_queue_.empty()) {
        CoroutineId next_id = waiting_queue_.front();
        waiting_queue_.pop();
        count_--; // 立即分配给下一个协程
        Scheduler::getInstance().resume(next_id);
    }
}

//=============================================================================
// Channel Template Implementation
//=============================================================================

template<typename T>
bool Channel<T>::send(const T& data) {
    if (closed_) {
        return false;
    }
    
    // 如果通道已满，等待
    if (capacity_ > 0 && buffer_.size() >= capacity_) {
        CoroutineId current_id = Scheduler::getInstance().getCurrentCoroutineId();
        if (current_id != 0) {
            send_queue_.push(current_id);
            Scheduler::getInstance().yield();
        }
        
        // 被唤醒后检查是否仍然可以发送
        if (closed_) {
            return false;
        }
    }
    
    // 发送数据
    buffer_.push(data);
    
    // 唤醒等待接收的协程
    if (!recv_queue_.empty()) {
        CoroutineId next_id = recv_queue_.front();
        recv_queue_.pop();
        Scheduler::getInstance().resume(next_id);
    }
    
    return true;
}

template<typename T>
bool Channel<T>::receive(T& data) {
    if (closed_ && buffer_.empty()) {
        return false;
    }
    
    // 如果通道为空，等待
    if (buffer_.empty()) {
        if (closed_) {
            return false;
        }
        
        CoroutineId current_id = Scheduler::getInstance().getCurrentCoroutineId();
        if (current_id != 0) {
            recv_queue_.push(current_id);
            Scheduler::getInstance().yield();
        }
        
        // 被唤醒后检查是否有数据
        if (buffer_.empty()) {
            return false;
        }
    }
    
    // 接收数据
    data = buffer_.front();
    buffer_.pop();
    
    // 唤醒等待发送的协程
    if (!send_queue_.empty()) {
        CoroutineId next_id = send_queue_.front();
        send_queue_.pop();
        Scheduler::getInstance().resume(next_id);
    }
    
    return true;
}

template<typename T>
bool Channel<T>::try_send(const T& data) {
    if (closed_) {
        return false;
    }
    
    if (capacity_ > 0 && buffer_.size() >= capacity_) {
        return false;
    }
    
    buffer_.push(data);
    
    // 唤醒等待接收的协程
    if (!recv_queue_.empty()) {
        CoroutineId next_id = recv_queue_.front();
        recv_queue_.pop();
        Scheduler::getInstance().resume(next_id);
    }
    
    return true;
}

template<typename T>
bool Channel<T>::try_receive(T& data) {
    if (buffer_.empty()) {
        return false;
    }
    
    data = buffer_.front();
    buffer_.pop();
    
    // 唤醒等待发送的协程
    if (!send_queue_.empty()) {
        CoroutineId next_id = send_queue_.front();
        send_queue_.pop();
        Scheduler::getInstance().resume(next_id);
    }
    
    return true;
}

template<typename T>
void Channel<T>::close() {
    closed_ = true;
    
    // 唤醒所有等待的协程
    while (!send_queue_.empty()) {
        CoroutineId next_id = send_queue_.front();
        send_queue_.pop();
        Scheduler::getInstance().resume(next_id);
    }
    
    while (!recv_queue_.empty()) {
        CoroutineId next_id = recv_queue_.front();
        recv_queue_.pop();
        Scheduler::getInstance().resume(next_id);
    }
}

// 显式实例化常用类型
template class Channel<int>;
template class Channel<std::string>;
template class Channel<void*>;
template class Channel<std::function<void()>>;

} // namespace coroutine 