# 分布式全局唯一ID生成器

本模块提供多种分布式全局唯一ID生成算法，适用于游戏服务器、微服务等分布式系统场景。通过统一接口和工厂方法，业务层可以灵活选择最适合的ID生成策略。

## 特性

- **统一接口**：`IIdGenerator` 抽象接口，支持字符串和数字两种格式的ID
- **多种实现**：UUID、雪花算法、时间戳+随机数等多种生成策略
- **线程安全**：所有实现都支持多线程并发调用
- **高性能**：雪花算法单机可达百万级QPS
- **易扩展**：可轻松添加新的ID生成算法

## 支持的ID生成算法

| 算法类型 | 特点 | 适用场景 | 性能 |
|----------|------|----------|------|
| UUID v4 | 完全随机，全球唯一 | 对ID格式无要求的场景 | 中等 |
| 雪花算法 | 趋势递增，高性能，可配置 | 游戏服务器，需要排序的场景 | 极高 |
| 时间戳 | 基于时间戳+计数器+随机数 | 简单场景，对性能要求不高 | 中等 |

## 算法详细说明

### 1. UUID v4 生成器

**特点**：
- 符合 RFC 4122 标准的UUID版本4
- 完全基于随机数生成，全球唯一
- 36字符格式：`xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx`
- 支持转换为64位数字（取前8字节）

**适用场景**：
- 用户ID、会话ID等不需要排序的场景
- 跨系统数据交换
- 对ID格式有标准要求的场景

**示例**：
```
字符串: 550e8400-e29b-41d4-a716-446655440000
数字:   6142341072123456789
```

### 2. 雪花算法（Snowflake）

**特点**：
- Twitter开源的分布式ID生成算法
- 64位整数，趋势递增，适合数据库主键
- 位组成：1位符号位 + 41位时间戳 + 5位数据中心ID + 5位工作节点ID + 12位序列号
- 理论QPS：每毫秒4096个ID，单机每秒400万+

**位结构**：
```
0 - 0000000000 0000000000 0000000000 0000000000 0 - 00000 - 00000 - 000000000000
|   |-------------------------------------------|   |-----|   |-----|   |----------|
符号位(1)        时间戳(41)                     数据中心(5) 工作节点(5)  序列号(12)
```

**配置参数**：
- `workerId`: 工作节点ID (0-1023)
- `datacenterId`: 数据中心ID (0-31)  
- `epoch`: 起始时间戳，默认2022-01-01

**适用场景**：
- 游戏服务器：玩家ID、道具ID、订单ID
- 数据库主键：需要排序和分页的场景
- 分布式系统：多节点部署，需要全局唯一

**优势**：
- 高性能：单机每秒400万+
- 趋势递增：适合数据库索引
- 时间有序：可从ID推算生成时间
- 分布式友好：不同节点生成的ID不冲突

### 3. 时间戳生成器

**特点**：
- 基于时间戳+计数器+随机数的组合
- 64位整数：42位时间戳 + 12位计数器 + 10位随机数
- 同一毫秒内通过计数器保证唯一性
- 实现简单，适合单机或小规模分布式

**位结构**：
```
|-------------------------------------------|-----------|----------|
              时间戳(42)                    计数器(12)   随机数(10)
```

**适用场景**：
- 单机应用或小规模集群
- 对性能要求不是极高的场景
- 临时ID、日志ID等

## 使用方法

### 基本使用

```cpp
#include "id_generator.h"
#include <iostream>

using namespace DistributedId;

int main() {
    // 创建不同类型的生成器
    auto uuidGen = createIdGenerator(IdGeneratorType::UUID);
    auto snowflakeGen = createIdGenerator(IdGeneratorType::Snowflake);
    auto timestampGen = createIdGenerator(IdGeneratorType::Timestamp);
    
    // 生成字符串ID
    std::cout << "UUID: " << uuidGen->generateStringId() << std::endl;
    std::cout << "Snowflake: " << snowflakeGen->generateStringId() << std::endl;
    std::cout << "Timestamp: " << timestampGen->generateStringId() << std::endl;
    
    // 生成数字ID
    std::cout << "UUID Numeric: " << uuidGen->generateNumericId() << std::endl;
    std::cout << "Snowflake Numeric: " << snowflakeGen->generateNumericId() << std::endl;
    std::cout << "Timestamp Numeric: " << timestampGen->generateNumericId() << std::endl;
    
    return 0;
}
```

### 雪花算法自定义配置

```cpp
// 自定义雪花算法配置
SnowflakeConfig config(
    1,                    // 工作节点ID
    1,                    // 数据中心ID  
    1640995200000ULL      // 起始时间戳（2022-01-01）
);

auto generator = createSnowflakeGenerator(config);
uint64_t id = generator->generateNumericId();
```

### 游戏服务器应用示例

```cpp
class GameServer {
private:
    std::unique_ptr<IIdGenerator> playerIdGen_;
    std::unique_ptr<IIdGenerator> itemIdGen_;
    std::unique_ptr<IIdGenerator> orderIdGen_;
    
public:
    GameServer(int serverId) {
        // 不同类型的ID使用不同的生成器
        SnowflakeConfig playerConfig(serverId, 1);  // 玩家ID
        SnowflakeConfig itemConfig(serverId, 2);    // 道具ID
        SnowflakeConfig orderConfig(serverId, 3);   // 订单ID
        
        playerIdGen_ = createSnowflakeGenerator(playerConfig);
        itemIdGen_ = createSnowflakeGenerator(itemConfig);
        orderIdGen_ = createSnowflakeGenerator(orderConfig);
    }
    
    uint64_t createPlayer() {
        uint64_t playerId = playerIdGen_->generateNumericId();
        // 创建玩家逻辑...
        return playerId;
    }
    
    uint64_t createItem(uint64_t playerId) {
        uint64_t itemId = itemIdGen_->generateNumericId();
        // 创建道具逻辑...
        return itemId;
    }
    
    uint64_t createOrder(uint64_t playerId) {
        uint64_t orderId = orderIdGen_->generateNumericId();
        // 创建订单逻辑...
        return orderId;
    }
};
```

## 性能测试

基于测试程序的性能数据（单线程）：

| 算法 | QPS | 内存占用 | CPU占用 | 特点 |
|------|-----|----------|---------|------|
| UUID | ~50万/秒 | 低 | 中等 | 完全随机 |
| 雪花算法 | ~100万/秒 | 极低 | 低 | 高性能，有序 |
| 时间戳 | ~80万/秒 | 低 | 低 | 简单实用 |

**并发性能**（4线程）：
- 雪花算法：4000个ID/ms，100%唯一性
- 线程安全：所有实现都支持多线程并发

## 部署建议

### 雪花算法部署配置

**单机部署**：
```cpp
SnowflakeConfig config(1, 1);  // 固定配置即可
```

**多机部署**：
```cpp
// 服务器1
SnowflakeConfig server1(1, 1);

// 服务器2  
SnowflakeConfig server2(2, 1);

// 服务器3
SnowflakeConfig server3(3, 1);
```

**多数据中心部署**：
```cpp
// 数据中心1，服务器1
SnowflakeConfig dc1_server1(1, 1);

// 数据中心1，服务器2
SnowflakeConfig dc1_server2(2, 1);

// 数据中心2，服务器1
SnowflakeConfig dc2_server1(1, 2);
```

### 配置管理建议

1. **配置文件管理**：将workerId和datacenterId写入配置文件
2. **自动分配**：通过注册中心自动分配节点ID
3. **环境区分**：开发/测试/生产环境使用不同的数据中心ID
4. **监控告警**：监控时钟回拨、ID重复等异常情况

## 编译和测试

```bash
# 编译
mkdir build && cd build
cmake ..
make

# 运行测试
./distributed_id/id_generator_test
```

## 扩展开发

要添加新的ID生成算法：

1. **继承接口**：
```cpp
class MyIdGenerator : public IIdGenerator {
    // 实现所有纯虚函数
};
```

2. **注册到工厂**：
```cpp
// 在 id_generator_factory.cpp 中添加
case IdGeneratorType::MyAlgorithm:
    return std::unique_ptr<IIdGenerator>(new MyIdGenerator());
```

3. **添加类型枚举**：
```cpp
// 在 id_generator.h 中添加
enum class IdGeneratorType {
    UUID, Snowflake, Timestamp, MyAlgorithm
};
```

## 注意事项

1. **时钟回拨**：雪花算法对系统时间敏感，部署时需要配置NTP同步
2. **节点ID管理**：确保不同节点的workerId和datacenterId不重复
3. **性能调优**：高并发场景下可考虑预生成ID池
4. **持久化**：重要业务场景建议将生成的ID持久化存储
5. **监控**：建议监控ID生成速率、重复率等指标

## 常见问题

**Q: 雪花算法的时间戳什么时候会用完？**
A: 41位时间戳可用约69年，从设定的epoch开始计算。

**Q: 如何处理时钟回拨？**
A: 当前实现会抛出异常，生产环境建议配置NTP或使用时钟回拨容错方案。

**Q: UUID转数字会有冲突吗？**
A: 理论上存在极小概率冲突，实际使用中概率可忽略。

**Q: 单机QPS不够怎么办？**
A: 可以考虑预生成ID池、分片生成、或使用专门的ID生成服务。

这个分布式ID生成器为游戏服务器提供了完整的ID管理解决方案，支持从单机到大规模分布式的各种部署场景。 