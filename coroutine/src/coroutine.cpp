/*
 * @Author       : GameServer
 * @Date         : 2024-01-01
 * @Description  : 协程库实现
 */

#include "../include/coroutine.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cassert>

#ifdef __APPLE__
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace coroutine {

// 当前调度器实例
static Scheduler* g_scheduler = nullptr;

// 协程入口函数的全局指针
static thread_local Coroutine* g_current_coroutine = nullptr;

//=============================================================================
// Coroutine Implementation
//=============================================================================

Coroutine::Coroutine(CoroutineId id, CoroutineFunc func, size_t stack_size)
    : id_(id), func_(std::move(func)), state_(CoroutineState::READY), 
      stack_(nullptr), stack_size_(stack_size) {
    
    // 分配栈空间
    stack_ = aligned_alloc(16, stack_size_);
    if (!stack_) {
        throw std::bad_alloc();
    }
    
    // 初始化栈空间
    std::memset(stack_, 0, stack_size_);
    
    // 初始化上下文
    std::memset(&context_, 0, sizeof(context_));
}

Coroutine::~Coroutine() {
    if (stack_) {
        std::free(stack_);
        stack_ = nullptr;
    }
}

void Coroutine::execute() {
    if (state_ != CoroutineState::READY && state_ != CoroutineState::SUSPENDED) {
        return;
    }
    
    state_ = CoroutineState::RUNNING;
    g_current_coroutine = this;
    
    try {
        func_();
    } catch (const std::exception& e) {
        std::cerr << "协程 " << id_ << " 执行异常: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "协程 " << id_ << " 执行未知异常" << std::endl;
    }
    
    state_ = CoroutineState::FINISHED;
    g_current_coroutine = nullptr;
}

//=============================================================================
// Scheduler Implementation
//=============================================================================

Scheduler::Scheduler() 
    : next_coroutine_id_(1), current_coroutine_id_(0), running_(false) {
    std::memset(&main_context_, 0, sizeof(main_context_));
}

Scheduler::~Scheduler() {
    stop();
}

Scheduler& Scheduler::getInstance() {
    if (!g_scheduler) {
        g_scheduler = new Scheduler();
    }
    return *g_scheduler;
}

CoroutineId Scheduler::createCoroutine(CoroutineFunc func, size_t stack_size) {
    CoroutineId id = next_coroutine_id_++;
    
    auto coroutine = std::unique_ptr<Coroutine>(new Coroutine(id, std::move(func), stack_size));
    coroutines_[id] = std::move(coroutine);
    ready_queue_.push(id);
    
    return id;
}

void Scheduler::resume(CoroutineId id) {
    auto it = coroutines_.find(id);
    if (it == coroutines_.end()) {
        std::cerr << "协程 " << id << " 不存在" << std::endl;
        return;
    }
    
    Coroutine* coroutine = it->second.get();
    if (coroutine->getState() == CoroutineState::FINISHED) {
        std::cerr << "协程 " << id << " 已完成" << std::endl;
        return;
    }
    
    if (coroutine->getState() == CoroutineState::SUSPENDED) {
        coroutine->setState(CoroutineState::READY);
        ready_queue_.push(id);
    }
}

void Scheduler::yield() {
    if (current_coroutine_id_ == 0) {
        return; // 主协程不能yield
    }
    
    auto it = coroutines_.find(current_coroutine_id_);
    if (it != coroutines_.end()) {
        Coroutine* coroutine = it->second.get();
        coroutine->setState(CoroutineState::SUSPENDED);
        
        // 保存当前上下文
        if (setjmp(*coroutine->getContext()) == 0) {
            // 跳转回主上下文
            longjmp(main_context_, 1);
        }
    }
}

void Scheduler::run() {
    running_ = true;
    
    // 保存主上下文
    if (setjmp(main_context_) == 0) {
        schedule();
    }
    
    // 调度循环
    while (running_ && !ready_queue_.empty()) {
        if (setjmp(main_context_) == 0) {
            schedule();
        }
    }
    
    running_ = false;
}

void Scheduler::stop() {
    running_ = false;
}

bool Scheduler::hasCoroutine(CoroutineId id) const {
    return coroutines_.find(id) != coroutines_.end();
}

void Scheduler::schedule() {
    if (ready_queue_.empty()) {
        return;
    }
    
    CoroutineId id = ready_queue_.front();
    ready_queue_.pop();
    
    auto it = coroutines_.find(id);
    if (it == coroutines_.end()) {
        return;
    }
    
    Coroutine* coroutine = it->second.get();
    current_coroutine_id_ = id;
    
    if (coroutine->getState() == CoroutineState::READY) {
        // 新协程，需要设置栈
        coroutineEntry();
    } else if (coroutine->getState() == CoroutineState::SUSPENDED) {
        // 恢复协程
        longjmp(*coroutine->getContext(), 1);
    }
    
    // 协程执行完毕或被挂起
    current_coroutine_id_ = 0;
    
    // 清理已完成的协程
    if (coroutine->getState() == CoroutineState::FINISHED) {
        coroutines_.erase(it);
    }
}

void Scheduler::coroutineEntry() {
    Scheduler& scheduler = getInstance();
    CoroutineId id = scheduler.getCurrentCoroutineId();
    
    auto it = scheduler.coroutines_.find(id);
    if (it != scheduler.coroutines_.end()) {
        Coroutine* coroutine = it->second.get();
        
        // 设置栈指针（简化实现，实际需要汇编代码）
        // char* stack_top = static_cast<char*>(coroutine->getStack()) + coroutine->getStackSize();
        
        // 在新栈上执行协程
        coroutine->execute();
        
        // 协程执行完毕，返回主上下文
        longjmp(scheduler.main_context_, 1);
    }
}

//=============================================================================
// API Implementation
//=============================================================================

namespace api {

CoroutineId create(CoroutineFunc func, size_t stack_size) {
    return Scheduler::getInstance().createCoroutine(std::move(func), stack_size);
}

void resume(CoroutineId id) {
    Scheduler::getInstance().resume(id);
}

void yield() {
    Scheduler::getInstance().yield();
}

void run() {
    Scheduler::getInstance().run();
}

void stop() {
    Scheduler::getInstance().stop();
}

CoroutineId current() {
    return Scheduler::getInstance().getCurrentCoroutineId();
}

} // namespace api

} // namespace coroutine 