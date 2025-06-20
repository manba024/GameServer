#ifndef UUID_GENERATOR_H
#define UUID_GENERATOR_H

#include "id_generator.h"
#include <random>
#include <mutex>

namespace DistributedId {

class UuidGenerator : public IIdGenerator {
public:
    UuidGenerator();
    
    std::string generateStringId() override;
    uint64_t generateNumericId() override;
    std::string getGeneratorInfo() const override;
    bool supportsNumericId() const override;

private:
    std::mt19937 rng_;
    std::mutex mutex_;
    
    // 生成UUID v4格式的字符串
    std::string generateUuidV4();
    
    // 将UUID转换为64位数字（取前8字节）
    uint64_t uuidToNumeric(const std::string& uuid);
};

} // namespace DistributedId

#endif // UUID_GENERATOR_H 