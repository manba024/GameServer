#include <iostream>
#include "my_string_utils.h"
#include <cstring>

using namespace mystringutils;

int main() {
    // 测试 my_memcpy
    char src1[] = "Hello, World!";
    char dest1[20] = {0};
    my_memcpy(dest1, src1, my_strlen(src1) + 1);
    std::cout << "my_memcpy: " << dest1 << std::endl;

    // 测试 my_memmove（重叠区域）
    char buf[] = "1234567890";
    my_memmove(buf + 2, buf, 8); // 将前8个字符向后移动2位
    std::cout << "my_memmove (overlap): " << buf << std::endl;

    // 测试 my_strcpy
    char src2[] = "abcdefg";
    char dest2[20] = {0};
    my_strcpy(dest2, src2);
    std::cout << "my_strcpy: " << dest2 << std::endl;

    // 测试 my_strncpy
    char src3[] = "hello";
    char dest3[10] = {0};
    my_strncpy(dest3, src3, 3);
    std::cout << "my_strncpy (3): " << dest3 << std::endl;

    // 测试 my_strlen
    std::cout << "my_strlen: " << my_strlen("test123") << std::endl;

    // 测试 my_strcmp
    std::cout << "my_strcmp('abc', 'abc'): " << my_strcmp("abc", "abc") << std::endl;
    std::cout << "my_strcmp('abc', 'abd'): " << my_strcmp("abc", "abd") << std::endl;
    std::cout << "my_strcmp('abd', 'abc'): " << my_strcmp("abd", "abc") << std::endl;

    return 0;
} 