#include <iostream>
#include "string_utils.h"
#include <cstring>
#include <cstddef>

int main() {
    // 测试 memcpy
    char src1[] = "Hello, World!";
    char dest1[20] = {0};
    MyStringUtils::memcpy(dest1, src1, MyStringUtils::strlen(src1) + 1);
    std::cout << "memcpy: " << dest1 << std::endl;

    // 测试 memmove（重叠区域）
    char buf[] = "1234567890";
    MyStringUtils::memmove(buf + 2, buf, 8); // 将前8个字符向后移动2位
    std::cout << "memmove (overlap): " << buf << std::endl;

    // 测试 strcpy
    char src2[] = "abcdefg";
    char dest2[20] = {0};
    MyStringUtils::strcpy(dest2, src2);
    std::cout << "strcpy: " << dest2 << std::endl;

    // 测试 strncpy
    char src3[] = "hello";
    char dest3[10] = {0};
    MyStringUtils::strncpy(dest3, src3, 3);
    std::cout << "strncpy (3): " << dest3 << std::endl;

    // 测试 strlen
    std::cout << "strlen: " << MyStringUtils::strlen("test123") << std::endl;

    // 测试 strcmp
    std::cout << "strcmp('abc', 'abc'): " << MyStringUtils::strcmp("abc", "abc") << std::endl;
    std::cout << "strcmp('abc', 'abd'): " << MyStringUtils::strcmp("abc", "abd") << std::endl;
    std::cout << "strcmp('abd', 'abc'): " << MyStringUtils::strcmp("abd", "abc") << std::endl;

    return 0;
} 