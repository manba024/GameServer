#pragma once

#include "io_multiplexer.h"
#include <memory>

namespace IOMultiplexing {

// IO复用器工厂
class MultiplexerFactory {
public:
    // 创建IO复用器
    static std::unique_ptr<IOMultiplexer> create(MultiplexerType type, 
                                                 int maxEvents = 1024);
    
    // 创建最适合当前平台的IO复用器
    static std::unique_ptr<IOMultiplexer> createBest(int maxEvents = 1024);
    
    // 获取平台支持的IO复用器类型
    static std::vector<MultiplexerType> getSupportedTypes();
    
    // 检查某种类型是否被支持
    static bool isSupported(MultiplexerType type);
    
    // 获取类型名称
    static std::string getTypeName(MultiplexerType type);
    
    // 获取推荐的类型（按性能排序）
    static std::vector<MultiplexerType> getRecommendedTypes();

private:
    MultiplexerFactory() = default; // 静态工厂，不允许实例化
};

// 便捷创建函数
std::unique_ptr<IOMultiplexer> createMultiplexer(
    MultiplexerType type = MultiplexerType::Select,
    int maxEvents = 1024
);

// 创建最优的IO复用器
std::unique_ptr<IOMultiplexer> createBestMultiplexer(int maxEvents = 1024);

} // namespace IOMultiplexing 