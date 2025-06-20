#ifndef SNOWFLAKE_GENERATOR_H
#define SNOWFLAKE_GENERATOR_H

#include "id_generator.h"
#include <mutex>
#include <chrono>

namespace DistributedId {

class SnowflakeGenerator : public IIdGenerator {
public:
    explicit SnowflakeGenerator(const SnowflakeConfig& config);
    
    std::string generateStringId() override;
    uint64_t generateNumericId() override;
    std::string getGeneratorInfo() const override;
    bool supportsNumericId() const override;

private:
    // 雪花算法位数分配
    static const uint64_t TIMESTAMP_BITS = 41;  // 时间戳位数
    static const uint64_t DATACENTER_BITS = 5;  // 数据中心位数
    static const uint64_t WORKER_BITS = 5;      // 工作节点位数
    static const uint64_t SEQUENCE_BITS = 12;   // 序列号位数
    
    // 最大值计算
    static const uint64_t MAX_DATACENTER_ID = (1ULL << DATACENTER_BITS) - 1;
    static const uint64_t MAX_WORKER_ID = (1ULL << WORKER_BITS) - 1;
    static const uint64_t MAX_SEQUENCE = (1ULL << SEQUENCE_BITS) - 1;
    
    // 位移量
    static const uint64_t WORKER_SHIFT = SEQUENCE_BITS;
    static const uint64_t DATACENTER_SHIFT = SEQUENCE_BITS + WORKER_BITS;
    static const uint64_t TIMESTAMP_SHIFT = SEQUENCE_BITS + WORKER_BITS + DATACENTER_BITS;
    
    SnowflakeConfig config_;
    uint64_t lastTimestamp_;
    uint64_t sequence_;
    std::mutex mutex_;
    
    // 获取当前时间戳（毫秒）
    uint64_t getCurrentTimestamp() const;
    
    // 等待下一毫秒
    uint64_t waitNextMillis(uint64_t lastTimestamp) const;
    
    // 验证配置参数
    void validateConfig(const SnowflakeConfig& config);
};

} // namespace DistributedId

#endif // SNOWFLAKE_GENERATOR_H 