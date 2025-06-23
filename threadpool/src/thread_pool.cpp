#include "../include/thread_pool.h"
#include <sstream>
#include <iomanip>

namespace ThreadPool {

std::string ThreadPoolStats::toString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "线程池统计信息:\n";
    oss << "  线程数量: " << threadCount << "\n";
    oss << "  活跃线程: " << activeThreads << "\n";
    oss << "  队列长度: " << queueSize << "/" << maxQueueSize << "\n";
    oss << "  已完成任务: " << completedTasks << "\n";
    oss << "  被拒绝任务: " << rejectedTasks << "\n";
    oss << "  平均执行时间: " << avgExecutionTime << "ms";
    return oss.str();
}

} // namespace ThreadPool 