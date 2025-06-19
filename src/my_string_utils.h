#ifndef MY_STRING_UTILS_H
#define MY_STRING_UTILS_H

#include <cstddef>

namespace mystringutils {

void *my_memcpy(void *dest, const void *src, std::size_t n);
void *my_memmove(void *dest, const void *src, std::size_t n);
char *my_strcpy(char *dest, const char *src);
char *my_strncpy(char *dest, const char *src, std::size_t n);
std::size_t my_strlen(const char *str);
int my_strcmp(const char *s1, const char *s2);

} // namespace mystringutils

#endif // MY_STRING_UTILS_H 