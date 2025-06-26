#include "../include/rpc_types.h"
#include <iostream>

using namespace rpc;

int main() {
    std::cout << "=== AnyValue测试 ===" << std::endl;
    
    try {
        // 测试字符串
        std::cout << "测试字符串..." << std::endl;
        AnyValue str_value("Hello World");
        std::cout << "字符串值: " << str_value.cast<std::string>() << std::endl;
        
        // 测试拷贝构造函数
        std::cout << "测试拷贝构造函数..." << std::endl;
        AnyValue copy_value(str_value);
        std::cout << "拷贝构造成功" << std::endl;
        
        // 测试拷贝的值
        std::cout << "测试拷贝的值..." << std::endl;
        std::cout << "拷贝值: " << copy_value.cast<std::string>() << std::endl;
        
        std::cout << "所有测试通过!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 