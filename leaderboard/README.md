# Leaderboard 排行榜多实现方案

本排行榜模块支持多种实现，适配不同用户量级和性能需求。通过统一接口和工厂方法，业务层可灵活切换实现。

## 统一接口
- `ILeaderboard`：排行榜抽象接口，支持添加/更新、删除、查询排名、查询分数、获取TopN。
- 工厂方法 `createLeaderboard(LeaderboardType type)` 创建不同实现。

## 已实现方案

| 实现方式         | 适用用户量级      | 时间复杂度                | 主要特点                                |
|------------------|-------------------|---------------------------|-----------------------------------------|
| Vector（顺序表） | 100~1,000         | 插入O(N)，查找O(N)        | 简单，内存连续，适合小规模               |
| Set（有序集合）  | 1,000~10,000      | 插入O(logN)，查分O(1)     | 自动排序，平衡二叉树，分数查询快速       |
| Heap（堆）       | 10,000~100,000    | 插入O(logN)，TopN O(NlogN)| 适合频繁TopN查询，内存效率高             |
| SkipList（跳表） | 10,000~1,000,000+ | 插入O(logN)，查找O(logN)  | Redis同款，支持范围查询，概率平衡         |

## 各实现详细说明

### 1. Vector 实现
**特点**：
- 基于 `std::vector` 存储，每次操作后重新排序
- 内存连续，缓存友好
- 实现简单，代码量少

**适用场景**：
- 用户数量少于1000
- 更新频率不高
- 对内存使用要求严格

**性能**：
- 添加/更新：O(N) + 排序O(NlogN)
- 删除：O(N)
- TopN：O(1)（已排序）
- 排名查询：O(N)

### 2. Set 实现
**特点**：
- 基于 `std::set` 自动维护排序
- 使用 `std::unordered_map` 快速查找用户分数
- 平衡二叉树结构，稳定性好

**适用场景**：
- 用户数量 1,000~10,000
- 需要稳定的性能表现
- 对排序一致性要求高
- 频繁查询用户分数

**性能**：
- 添加/更新：O(logN)
- 删除：O(logN)
- TopN：O(N)（遍历前N个）
- 排名查询：O(N)（遍历计数）
- 分数查询：O(1)（unordered_map 直接查找）

### 3. Heap 实现
**特点**：
- 基于 `std::priority_queue` 最大堆
- 使用延迟删除策略（标记删除）
- 定期清理无效条目

**适用场景**：
- 用户数量 10,000~100,000
- 频繁查询TopN
- 删除操作相对较少

**性能**：
- 添加/更新：O(logN)
- 删除：O(1)（标记删除）
- TopN：O(NlogN)（复制堆并弹出）
- 排名查询：O(NlogN)（遍历堆）

**注意**：删除操作使用标记删除，需要定期清理堆以避免内存膨胀。

### 4. SkipList 实现
**特点**：
- 自实现跳表数据结构
- 概率平衡，平均性能接近平衡树
- 支持高效的范围查询和排名查询

**适用场景**：
- 用户数量 10,000~1,000,000+
- 需要高性能的排名查询
- 对内存使用有一定要求

**性能**：
- 添加/更新：O(logN)
- 删除：O(logN)
- TopN：O(N)（遍历前N个）
- 排名查询：O(N)（遍历计数）

**实现细节**：
- 最大层数：16层
- 随机层数生成：50%概率向上增加层数
- 头节点使用哨兵值（INT_MIN）

## 分布式排行榜方案

### 适用场景
- 用户量级达到百万、千万级，单机内存无法承载全部数据
- 需要多服/跨服排行榜
- 需要高可用、横向扩展、数据持久化

### 常见架构
- **分片（Sharding）**：按用户ID或分数区间分片
- **分布式缓存+数据库**：Redis Cluster + MySQL/PostgreSQL
- **消息队列/异步同步**：最终一致性
- **多级排行榜**：本地TopN+全局TopN

### 技术选型
- **Redis ZSet/SortedSet**：高性能、支持分布式、原子操作
- **HBase/ClickHouse/Elasticsearch**：超大数据量、复杂查询
- **自研分布式服务**：定制化需求

## 工厂方法

```cpp
std::unique_ptr<ILeaderboard> createLeaderboard(LeaderboardType type);
```

支持的类型：
- `LeaderboardType::Vector` - 基于 std::vector 实现
- `LeaderboardType::Set` - 基于 std::set 实现  
- `LeaderboardType::Heap` - 基于 std::priority_queue 实现
- `LeaderboardType::SkipList` - 基于跳表实现

## 编译和测试

```bash
# 编译
mkdir build && cd build
cmake ..
make

# 运行功能测试
./leaderboard_test

# 运行性能对比测试
./test_all_implementations
```

## 使用示例

```cpp
#include "leaderboard.h"
#include <iostream>

int main() {
    // 创建不同类型的排行榜
    auto vectorLB = Leaderboard::createLeaderboard(Leaderboard::LeaderboardType::Vector);
    auto setLB = Leaderboard::createLeaderboard(Leaderboard::LeaderboardType::Set);
    auto heapLB = Leaderboard::createLeaderboard(Leaderboard::LeaderboardType::Heap);
    auto skiplistLB = Leaderboard::createLeaderboard(Leaderboard::LeaderboardType::SkipList);
    
    // 添加用户
    vectorLB->addOrUpdate("Alice", 100);
    vectorLB->addOrUpdate("Bob", 85);
    vectorLB->addOrUpdate("Charlie", 120);
    
    // 查询TopN
    auto top3 = vectorLB->topN(3);
    for (const auto& entry : top3) {
        std::cout << entry.userId << ": " << entry.score << std::endl;
    }
    
    // 查询排名
    std::cout << "Alice排名: " << vectorLB->getRank("Alice") << std::endl;
    
    return 0;
}
```

## 性能选择建议

- **100~1,000 用户**：Vector - 简单高效，内存占用小
- **1,000~10,000 用户**：Set - 稳定性能，自动排序
- **10,000~100,000 用户**：Heap（TopN频繁）或 SkipList（排名查询频繁）
- **100,000+ 用户**：SkipList 或考虑分布式方案

## 扩展说明

要添加新的实现：
1. 继承 `ILeaderboard` 接口
2. 实现所有纯虚函数
3. 在工厂方法中注册新类型
4. 业务代码无需修改，只需切换工厂参数

这种设计使得系统可以根据实际需求灵活选择最适合的排行榜实现。 

## 性能特点对比

| 实现 | 添加/更新 | 删除 | TopN | 排名查询 | 分数查询 | 内存效率 | 适用场景 |
|------|-----------|------|------|----------|----------|----------|----------|
| Vector | O(NlogN) | O(N) | O(1) | O(N) | O(N) | 最高 | 小规模，内存敏感 |
| Set | O(logN) | O(logN) | O(N) | O(N) | **O(1)** | 高 | 中等规模，频繁查分数 |
| Heap | O(logN) | O(1) | O(NlogN) | O(NlogN) | O(1) | 中 | TopN频繁，删除少 |
| SkipList | O(logN) | O(logN) | O(N) | O(N) | O(logN) | 中 | 大规模，全功能 | 