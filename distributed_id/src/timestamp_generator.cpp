#include "../include/timestamp_generator.h"
#include <sstream>
#include <chrono>

namespace DistributedId {

TimestampGenerator::TimestampGenerator() 
    : rng_(std::random_device{}()), counter_(0), lastTimestamp_(0) {
}

std::string TimestampGenerator::generateStringId() {
    uint64_t id = generateNumericId();
    return std::to_string(id);
}

uint64_t TimestampGenerator::generateNumericId() {
    return generateTimestampBasedId();
}

std::string TimestampGenerator::getGeneratorInfo() const {
    return "Timestamp Generator - 基于时间戳+计数器+随机数的ID生成器";
}

bool TimestampGenerator::supportsNumericId() const {
    return true;
}

uint64_t TimestampGenerator::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

uint64_t TimestampGenerator::generateTimestampBasedId() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    uint64_t timestamp = getCurrentTimestamp();
    
    if (timestamp == lastTimestamp_) {
        // 同一毫秒内，计数器递增
        counter_++;
        if (counter_ >= 4096) { // 12位计数器最大值
            counter_ = 0;
        }
    } else {
        // 新的毫秒，计数器重置
        counter_ = 0;
        lastTimestamp_ = timestamp;
    }
    
    // 生成随机数 (12位)
    std::uniform_int_distribution<uint32_t> dis(0, 4095);
    uint32_t randomPart = dis(rng_);
    
    // 组装ID: [时间戳42位][计数器12位][随机数10位]
    // 时间戳取低42位，计数器12位，随机数10位
    uint64_t id = ((timestamp & 0x3FFFFFFFFFF) << 22) |  // 时间戳42位
                  ((counter_ & 0xFFF) << 10) |           // 计数器12位
                  (randomPart & 0x3FF);                  // 随机数10位
    
    return id;
}

} // namespace DistributedId 