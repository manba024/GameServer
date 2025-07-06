/*
 * @Author       : GameServer
 * @Date         : 2024-01-01
 * @Description  : 协程库高级使用示例
 */

#include "../include/coroutine.h"
#include "../include/sync.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace coroutine;

// 协程池示例
class CoroutinePool {
private:
    std::vector<CoroutineId> workers_;
    Channel<std::function<void()>> task_queue_;
    bool running_;
    
public:
    CoroutinePool(int worker_count = 4) : task_queue_(100), running_(false) {
        for (int i = 0; i < worker_count; ++i) {
            CoroutineId id = api::create([this, i]() { worker(i); });
            workers_.push_back(id);
        }
    }
    
    void start() {
        running_ = true;
        std::cout << "协程池启动，包含 " << workers_.size() << " 个工作协程" << std::endl;
    }
    
    void stop() {
        running_ = false;
        task_queue_.close();
        std::cout << "协程池停止" << std::endl;
    }
    
    void submit(std::function<void()> task) {
        if (running_) {
            task_queue_.send(task);
        }
    }
    
private:
    void worker(int id) {
        std::cout << "工作协程 " << id << " 启动" << std::endl;
        
        std::function<void()> task;
        while (running_ && task_queue_.receive(task)) {
            std::cout << "工作协程 " << id << " 执行任务" << std::endl;
            try {
                task();
            } catch (const std::exception& e) {
                std::cout << "工作协程 " << id << " 任务执行异常: " << e.what() << std::endl;
            }
            api::yield();
        }
        
        std::cout << "工作协程 " << id << " 结束" << std::endl;
    }
};

// 异步HTTP请求模拟
class HttpClient {
private:
    Channel<std::string> response_channel_;
    
public:
    HttpClient() : response_channel_(10) {}
    
    void async_get(const std::string& url) {
        api::create([this, url]() {
            // 模拟网络请求
            std::cout << "发起HTTP请求: " << url << std::endl;
            
            // 模拟网络延迟
            for (int i = 0; i < 3; ++i) {
                api::yield();
            }
            
            // 模拟响应
            std::string response = "HTTP/1.1 200 OK\nContent: Response from " + url;
            response_channel_.send(response);
        });
    }
    
    std::string wait_response() {
        std::string response;
        if (response_channel_.receive(response)) {
            return response;
        }
        return "";
    }
};

// 生产者-消费者模式的任务调度器
class TaskScheduler {
private:
    Channel<std::string> task_queue_;
    std::vector<CoroutineId> workers_;
    bool running_;
    
public:
    TaskScheduler(int worker_count = 3) : task_queue_(20), running_(false) {
        for (int i = 0; i < worker_count; ++i) {
            CoroutineId id = api::create([this, i]() { worker(i); });
            workers_.push_back(id);
        }
    }
    
    void start() {
        running_ = true;
        std::cout << "任务调度器启动" << std::endl;
    }
    
    void stop() {
        running_ = false;
        task_queue_.close();
        std::cout << "任务调度器停止" << std::endl;
    }
    
    void add_task(const std::string& task) {
        if (running_) {
            task_queue_.send(task);
        }
    }
    
private:
    void worker(int id) {
        std::cout << "任务处理器 " << id << " 启动" << std::endl;
        
        std::string task;
        while (running_ && task_queue_.receive(task)) {
            std::cout << "任务处理器 " << id << " 处理任务: " << task << std::endl;
            
            // 模拟任务处理时间
            for (int i = 0; i < 2; ++i) {
                api::yield();
            }
            
            std::cout << "任务处理器 " << id << " 完成任务: " << task << std::endl;
        }
        
        std::cout << "任务处理器 " << id << " 结束" << std::endl;
    }
};

// 协程管道示例
void pipeline_example() {
    std::cout << "\n--- 协程管道示例 ---" << std::endl;
    
    Channel<int> input_channel(5);
    Channel<int> output_channel(5);
    
    // 数据生成器
    api::create([&input_channel]() {
        std::cout << "数据生成器启动" << std::endl;
        for (int i = 1; i <= 10; ++i) {
            input_channel.send(i);
            std::cout << "生成数据: " << i << std::endl;
            api::yield();
        }
        input_channel.close();
        std::cout << "数据生成器结束" << std::endl;
    });
    
    // 数据处理器
    api::create([&input_channel, &output_channel]() {
        std::cout << "数据处理器启动" << std::endl;
        int data;
        while (input_channel.receive(data)) {
            int processed = data * data; // 平方处理
            output_channel.send(processed);
            std::cout << "处理数据: " << data << " -> " << processed << std::endl;
            api::yield();
        }
        output_channel.close();
        std::cout << "数据处理器结束" << std::endl;
    });
    
    // 数据消费者
    api::create([&output_channel]() {
        std::cout << "数据消费者启动" << std::endl;
        int data;
        while (output_channel.receive(data)) {
            std::cout << "消费数据: " << data << std::endl;
            api::yield();
        }
        std::cout << "数据消费者结束" << std::endl;
    });
    
    api::run();
}

// 协程间通信示例
void communication_example() {
    std::cout << "\n--- 协程间通信示例 ---" << std::endl;
    
    Channel<std::string> message_channel(10);
    
    // 消息发送者
    for (int i = 1; i <= 3; ++i) {
        api::create([&message_channel, i]() {
            for (int j = 1; j <= 3; ++j) {
                std::ostringstream oss;
                oss << "消息来自协程" << i << "-" << j;
                message_channel.send(oss.str());
                std::cout << "发送: " << oss.str() << std::endl;
                api::yield();
            }
        });
    }
    
    // 消息接收者
    api::create([&message_channel]() {
        std::string message;
        int count = 0;
        while (count < 9 && message_channel.receive(message)) {
            std::cout << "接收: " << message << std::endl;
            count++;
            api::yield();
        }
    });
    
    api::run();
}

int main() {
    std::cout << "=== 协程库高级示例程序 ===" << std::endl;
    
    // 示例1: 协程池
    std::cout << "\n--- 示例1: 协程池 ---" << std::endl;
    CoroutinePool pool(3);
    pool.start();
    
    // 提交任务
    for (int i = 1; i <= 5; ++i) {
        pool.submit([i]() {
            std::cout << "执行任务 " << i << std::endl;
            // 模拟任务处理
            for (int j = 0; j < 2; ++j) {
                api::yield();
            }
            std::cout << "任务 " << i << " 完成" << std::endl;
        });
    }
    
    api::run();
    pool.stop();
    
    // 示例2: 异步HTTP客户端
    std::cout << "\n--- 示例2: 异步HTTP客户端 ---" << std::endl;
    HttpClient client;
    
    // 发起多个异步请求
    client.async_get("http://example.com/api1");
    client.async_get("http://example.com/api2");
    client.async_get("http://example.com/api3");
    
    // 等待响应
    api::create([&client]() {
        for (int i = 0; i < 3; ++i) {
            std::string response = client.wait_response();
            std::cout << "收到响应: " << response << std::endl;
        }
    });
    
    api::run();
    
    // 示例3: 任务调度器
    std::cout << "\n--- 示例3: 任务调度器 ---" << std::endl;
    TaskScheduler scheduler(2);
    scheduler.start();
    
    // 添加任务
    for (int i = 1; i <= 6; ++i) {
        std::ostringstream oss;
        oss << "任务-" << i;
        scheduler.add_task(oss.str());
    }
    
    api::run();
    scheduler.stop();
    
    // 示例4: 协程管道
    pipeline_example();
    
    // 示例5: 协程间通信
    communication_example();
    
    std::cout << "\n=== 所有高级示例执行完成 ===" << std::endl;
    return 0;
} 