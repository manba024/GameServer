/*
 * @Author       : GameServer
 * @Date         : 2024-01-01
 * @Description  : 协程库主头文件
 */

#ifndef COROUTINE_H
#define COROUTINE_H

#include <functional>
#include <memory>
#include <vector>
#include <queue>
#include <map>
#include <setjmp.h>
#include <cstdint>

namespace coroutine {

// 协程状态枚举
enum class CoroutineState {
    READY,      // 就绪态
    RUNNING,    // 运行态
    SUSPENDED,  // 挂起态
    FINISHED    // 完成态
};

// 协程ID类型
using CoroutineId = uint64_t;

// 协程函数类型
using CoroutineFunc = std::function<void()>;

// 前向声明
class Scheduler;

/**
 * @brief 协程上下文类
 * 
 * 管理单个协程的执行上下文，包括栈空间、寄存器状态等
 */
class Coroutine {
public:
    static const size_t DEFAULT_STACK_SIZE = 1024 * 128; // 128KB默认栈大小
    
    /**
     * @brief 构造函数
     * @param id 协程ID
     * @param func 协程执行函数
     * @param stack_size 栈大小
     */
    Coroutine(CoroutineId id, CoroutineFunc func, size_t stack_size = DEFAULT_STACK_SIZE);
    
    /**
     * @brief 析构函数
     */
    ~Coroutine();
    
    /**
     * @brief 获取协程ID
     */
    CoroutineId getId() const { return id_; }
    
    /**
     * @brief 获取协程状态
     */
    CoroutineState getState() const { return state_; }
    
    /**
     * @brief 设置协程状态
     */
    void setState(CoroutineState state) { state_ = state; }
    
    /**
     * @brief 获取栈指针
     */
    void* getStack() const { return stack_; }
    
    /**
     * @brief 获取栈大小
     */
    size_t getStackSize() const { return stack_size_; }
    
    /**
     * @brief 获取上下文
     */
    jmp_buf* getContext() { return &context_; }
    
    /**
     * @brief 执行协程函数
     */
    void execute();
    
private:
    CoroutineId id_;                    // 协程ID
    CoroutineFunc func_;                // 协程函数
    CoroutineState state_;              // 协程状态
    void* stack_;                       // 栈空间
    size_t stack_size_;                 // 栈大小
    jmp_buf context_;                   // 上下文
    
    // 禁用拷贝构造和赋值
    Coroutine(const Coroutine&) = delete;
    Coroutine& operator=(const Coroutine&) = delete;
};

/**
 * @brief 协程调度器类
 * 
 * 负责协程的创建、调度和管理
 */
class Scheduler {
public:
    /**
     * @brief 获取调度器单例
     */
    static Scheduler& getInstance();
    
    /**
     * @brief 创建协程
     * @param func 协程函数
     * @param stack_size 栈大小
     * @return 协程ID
     */
    CoroutineId createCoroutine(CoroutineFunc func, size_t stack_size = Coroutine::DEFAULT_STACK_SIZE);
    
    /**
     * @brief 启动协程
     * @param id 协程ID
     */
    void resume(CoroutineId id);
    
    /**
     * @brief 挂起当前协程
     */
    void yield();
    
    /**
     * @brief 运行调度器
     */
    void run();
    
    /**
     * @brief 停止调度器
     */
    void stop();
    
    /**
     * @brief 获取当前运行的协程ID
     */
    CoroutineId getCurrentCoroutineId() const { return current_coroutine_id_; }
    
    /**
     * @brief 检查协程是否存在
     */
    bool hasCoroutine(CoroutineId id) const;
    
    /**
     * @brief 获取协程数量
     */
    size_t getCoroutineCount() const { return coroutines_.size(); }
    
private:
    Scheduler();
    ~Scheduler();
    
    // 禁用拷贝构造和赋值
    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    
    std::map<CoroutineId, std::unique_ptr<Coroutine>> coroutines_;  // 协程映射
    std::queue<CoroutineId> ready_queue_;                           // 就绪队列
    CoroutineId next_coroutine_id_;                                 // 下一个协程ID
    CoroutineId current_coroutine_id_;                              // 当前协程ID
    jmp_buf main_context_;                                          // 主上下文
    bool running_;                                                  // 运行标志
    
    /**
     * @brief 调度下一个协程
     */
    void schedule();
    
    /**
     * @brief 协程入口函数
     */
    static void coroutineEntry();
};

// 全局函数接口
namespace api {
    /**
     * @brief 创建协程
     */
    CoroutineId create(CoroutineFunc func, size_t stack_size = Coroutine::DEFAULT_STACK_SIZE);
    
    /**
     * @brief 启动协程
     */
    void resume(CoroutineId id);
    
    /**
     * @brief 挂起当前协程
     */
    void yield();
    
    /**
     * @brief 运行调度器
     */
    void run();
    
    /**
     * @brief 停止调度器
     */
    void stop();
    
    /**
     * @brief 获取当前协程ID
     */
    CoroutineId current();
}

} // namespace coroutine

#endif // COROUTINE_H 