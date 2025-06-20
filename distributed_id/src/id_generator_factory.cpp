#include "../include/id_generator.h"
#include "../include/uuid_generator.h"
#include "../include/snowflake_generator.h"
#include "../include/timestamp_generator.h"

namespace DistributedId {

std::unique_ptr<IIdGenerator> createIdGenerator(IdGeneratorType type) {
    switch (type) {
        case IdGeneratorType::UUID:
            return std::unique_ptr<IIdGenerator>(new UuidGenerator());
        case IdGeneratorType::Snowflake:
            // 使用默认配置
            return std::unique_ptr<IIdGenerator>(new SnowflakeGenerator(SnowflakeConfig()));
        case IdGeneratorType::Timestamp:
            return std::unique_ptr<IIdGenerator>(new TimestampGenerator());
        default:
            return nullptr;
    }
}

std::unique_ptr<IIdGenerator> createSnowflakeGenerator(const SnowflakeConfig& config) {
    return std::unique_ptr<IIdGenerator>(new SnowflakeGenerator(config));
}

} // namespace DistributedId 