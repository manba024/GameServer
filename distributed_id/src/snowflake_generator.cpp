#include "../include/snowflake_generator.h"
#include <stdexcept>
#include <sstream>
#include <thread>

namespace DistributedId {

SnowflakeGenerator::SnowflakeGenerator(const SnowflakeConfig& config) 
    : config_(config), lastTimestamp_(0), sequence_(0) {
    validateConfig(config);
}

std::string SnowflakeGenerator::generateStringId() {
    uint64_t id = generateNumericId();
    return std::to_string(id);
}

uint64_t SnowflakeGenerator::generateNumericId() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    uint64_t timestamp = getCurrentTimestamp();
    
    // 时间回拨检查
    if (timestamp < lastTimestamp_) {
        throw std::runtime_error("时钟回拨错误，拒绝生成ID");
    }
    
    if (timestamp == lastTimestamp_) {
        // 同一毫秒内，序列号递增
        sequence_ = (sequence_ + 1) & MAX_SEQUENCE;
        if (sequence_ == 0) {
            // 序列号用完，等待下一毫秒
            timestamp = waitNextMillis(lastTimestamp_);
        }
    } else {
        // 新的毫秒，序列号重置
        sequence_ = 0;
    }
    
    lastTimestamp_ = timestamp;
    
    // 组装64位ID
    uint64_t id = ((timestamp - config_.epoch) << TIMESTAMP_SHIFT) |
                  (static_cast<uint64_t>(config_.datacenterId) << DATACENTER_SHIFT) |
                  (static_cast<uint64_t>(config_.workerId) << WORKER_SHIFT) |
                  sequence_;
    
    return id;
}

std::string SnowflakeGenerator::getGeneratorInfo() const {
    std::ostringstream oss;
    oss << "Snowflake Generator - WorkerID:" << config_.workerId 
        << ", DatacenterID:" << config_.datacenterId 
        << ", Epoch:" << config_.epoch;
    return oss.str();
}

bool SnowflakeGenerator::supportsNumericId() const {
    return true; // 雪花算法原生支持64位数字ID
}

uint64_t SnowflakeGenerator::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

uint64_t SnowflakeGenerator::waitNextMillis(uint64_t lastTimestamp) const {
    uint64_t timestamp = getCurrentTimestamp();
    while (timestamp <= lastTimestamp) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        timestamp = getCurrentTimestamp();
    }
    return timestamp;
}

void SnowflakeGenerator::validateConfig(const SnowflakeConfig& config) {
    if (config.workerId > MAX_WORKER_ID) {
        throw std::invalid_argument("WorkerID超出范围 [0, " + std::to_string(MAX_WORKER_ID) + "]");
    }
    if (config.datacenterId > MAX_DATACENTER_ID) {
        throw std::invalid_argument("DatacenterID超出范围 [0, " + std::to_string(MAX_DATACENTER_ID) + "]");
    }
    
    uint64_t currentTime = getCurrentTimestamp();
    if (config.epoch > currentTime) {
        throw std::invalid_argument("起始时间戳不能大于当前时间");
    }
}

} // namespace DistributedId 