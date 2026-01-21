#include <valen/string.h>
#include <stddef.h>

void *memset(void *ptr, int value, uint64_t num) {
    uint8_t *p = (uint8_t *)ptr;
    while (num--) {
        *p++ = (uint8_t)value;
    }
    return ptr;
}

void *memcpy(void *dest, const void *src, uint64_t num) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (num--) {
        *d++ = *s++;
    }
    return dest;
}

int strlen(const char *str) {
    int len = 0;
    while (*str++) {
        len++;
    }
    return len;
}

int strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(const unsigned char *)str1 - *(const unsigned char *)str2;
}

int strncmp(const char *str1, const char *str2, uint64_t n) {
    while (n-- && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return n == (uint64_t)-1 ? 0 : *(const unsigned char *)str1 - *(const unsigned char *)str2;
}

char *strchr(const char *str, int c) {
    while (*str) {
        if (*str == (char)c) {
            return (char *)str;
        }
        str++;
    }
    return NULL;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

char *strncpy(char *dest, const char *src, uint64_t n) {
    uint64_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}
