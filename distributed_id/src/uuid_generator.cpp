#include "../include/uuid_generator.h"
#include <sstream>
#include <iomanip>
#include <string>

namespace DistributedId {

UuidGenerator::UuidGenerator() : rng_(std::random_device{}()) {
}

std::string UuidGenerator::generateStringId() {
    std::lock_guard<std::mutex> lock(mutex_);
    return generateUuidV4();
}

uint64_t UuidGenerator::generateNumericId() {
    std::string uuid = generateStringId();
    return uuidToNumeric(uuid);
}

std::string UuidGenerator::getGeneratorInfo() const {
    return "UUID v4 Generator - 随机生成符合RFC 4122标准的UUID";
}

bool UuidGenerator::supportsNumericId() const {
    return true; // 可以将UUID转换为64位数字
}

std::string UuidGenerator::generateUuidV4() {
    // UUID v4 格式: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    // 其中4表示版本号，y的第一位必须是8,9,A,B之一
    
    std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);
    std::uniform_int_distribution<uint16_t> dis16(0, 0xFFFF);
    std::uniform_int_distribution<uint8_t> dis8(0, 0xFF);
    
    uint32_t time_low = dis(rng_);
    uint16_t time_mid = dis16(rng_);
    uint16_t time_hi_and_version = (dis16(rng_) & 0x0FFF) | 0x4000; // 版本4
    uint8_t clock_seq_hi_and_reserved = (dis8(rng_) & 0x3F) | 0x80; // 变体位
    uint8_t clock_seq_low = dis8(rng_);
    
    // 生成6字节的node
    uint64_t node = 0;
    for (int i = 0; i < 6; i++) {
        node = (node << 8) | dis8(rng_);
    }
    
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(8) << time_low << "-"
        << std::setw(4) << time_mid << "-"
        << std::setw(4) << time_hi_and_version << "-"
        << std::setw(2) << static_cast<int>(clock_seq_hi_and_reserved)
        << std::setw(2) << static_cast<int>(clock_seq_low) << "-"
        << std::setw(12) << node;
    
    return oss.str();
}

uint64_t UuidGenerator::uuidToNumeric(const std::string& uuid) {
    // 提取UUID的前16个十六进制字符（去掉连字符）
    std::string hexStr;
    for (char c : uuid) {
        if (c != '-' && hexStr.length() < 16) {
            hexStr += c;
        }
    }
    
    // 转换为64位整数
    uint64_t result = 0;
    for (char c : hexStr) {
        result <<= 4;
        if (c >= '0' && c <= '9') {
            result |= (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            result |= (c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            result |= (c - 'A' + 10);
        }
    }
    
    return result;
}

} // namespace DistributedId 