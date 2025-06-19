#include "string_utils.h"
#include <cstddef>

namespace MyStringUtils {

void *memcpy(void *dest, const void *src, std::size_t n) {
    unsigned char *d = static_cast<unsigned char *>(dest);
    const unsigned char *s = static_cast<const unsigned char *>(src);
    for (std::size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
    return dest;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++)) {}
    return dest;
}

char *strncpy(char *dest, const char *src, std::size_t n) {
    std::size_t i = 0;
    for (; i < n && src[i] != '\0'; ++i) {
        dest[i] = src[i];
    }
    for (; i < n; ++i) {
        dest[i] = '\0';
    }
    return dest;
}

std::size_t strlen(const char *str) {
    std::size_t len = 0;
    while (str[len] != '\0') {
        ++len;
    }
    return len;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        ++s1;
        ++s2;
    }
    return static_cast<unsigned char>(*s1) - static_cast<unsigned char>(*s2);
}

// 地址本质上是数字，判断是否重叠之需要按照数字的方式判断区间是否重叠即可 
void *memmove(void *dest, const void *src, std::size_t n) {
    unsigned char *d = static_cast<unsigned char *>(dest);
    const unsigned char *s = static_cast<const unsigned char *>(src);
    if (d == s || n == 0) {
        return dest;
    }
    if (d < s || d >= s + n) {
        // 不重叠，正向拷贝
        for (std::size_t i = 0; i < n; ++i) {
            d[i] = s[i];
        }
    } else {
        // 有重叠，反向拷贝
        for (std::size_t i = n; i > 0; --i) {
            d[i - 1] = s[i - 1];
        }
    }
    return dest;
}

} // namespace MyStringUtils 