#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <cstddef>

namespace MyStringUtils {

void *memcpy(void *dest, const void *src, std::size_t n);
void *memmove(void *dest, const void *src, std::size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, std::size_t n);
std::size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);

} // namespace MyStringUtils

#endif // STRING_UTILS_H 