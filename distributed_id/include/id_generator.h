#ifndef ID_GENERATOR_H
#define ID_GENERATOR_H

#include <string>
#include <memory>
#include <cstdint>

namespace DistributedId {

// ID生成器类型
enum class IdGeneratorType {
    UUID,           // UUID v4 实现
    Snowflake,      // 雪花算法实现
    Timestamp       // 时间戳+随机数实现
};

// 统一的ID生成器接口
class IIdGenerator {
public:
    virtual ~IIdGenerator() = default;
    
    // 生成字符串形式的ID
    virtual std::string generateStringId() = 0;
    
    // 生成64位整数形式的ID（如果支持）
    virtual uint64_t generateNumericId() = 0;
    
    // 获取生成器类型信息
    virtual std::string getGeneratorInfo() const = 0;
    
    // 检查是否支持数字ID
    virtual bool supportsNumericId() const = 0;
};

// 雪花算法配置参数
struct SnowflakeConfig {
    uint16_t workerId;      // 工作节点ID (0-1023)
    uint16_t datacenterId;  // 数据中心ID (0-31)
    uint64_t epoch;         // 起始时间戳 (毫秒)
    
    SnowflakeConfig(uint16_t worker = 1, uint16_t datacenter = 1, uint64_t startEpoch = 1640995200000ULL)
        : workerId(worker), datacenterId(datacenter), epoch(startEpoch) {}
};

// 工厂方法
std::unique_ptr<IIdGenerator> createIdGenerator(IdGeneratorType type);
std::unique_ptr<IIdGenerator> createSnowflakeGenerator(const SnowflakeConfig& config);

} // namespace DistributedId

#endif // ID_GENERATOR_H 