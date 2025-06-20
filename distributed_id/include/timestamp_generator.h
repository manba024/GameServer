#ifndef TIMESTAMP_GENERATOR_H
#define TIMESTAMP_GENERATOR_H

#include "id_generator.h"
#include <random>
#include <mutex>

namespace DistributedId {

class TimestampGenerator : public IIdGenerator {
public:
    TimestampGenerator();
    
    std::string generateStringId() override;
    uint64_t generateNumericId() override;
    std::string getGeneratorInfo() const override;
    bool supportsNumericId() const override;

private:
    std::mt19937 rng_;
    std::mutex mutex_;
    uint32_t counter_;  // 同一毫秒内的计数器
    uint64_t lastTimestamp_;
    
    // 获取当前时间戳（毫秒）
    uint64_t getCurrentTimestamp() const;
    
    // 生成基于时间戳的ID
    uint64_t generateTimestampBasedId();
};

} // namespace DistributedId

#endif // TIMESTAMP_GENERATOR_H 