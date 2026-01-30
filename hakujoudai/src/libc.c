/*
 * Copyright (c) 2005-2018 Rich Felker
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <libc.h>

static inline int is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

static inline int is_xdigit(char c)
{
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

static inline int hexval(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return c - 'A' + 10;
}

size_t strlen(const char *str)
{
    const char *s;
    for (s = str; *s; ++s);
    return (size_t)(s - str);
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2++) {
        if (*s1++ == 0)
            return 0;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)--s2;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0)
        return 0;

    do {
        if (*s1 != *s2++)
            return *(const unsigned char *)s1 - *(const unsigned char *)--s2;
        if (*s1++ == 0)
            break;
    } while (--n != 0);

    return 0;
}

char *strstr(const char *s1, const char *s2)
{
    size_t n = strlen(s2);
    while (*s1) {
        if (!memcmp(s1, s2, n))
            return (char *)s1;
        s1++;
    }
    return 0;
}

void *memset(void *dst, int c, size_t n)
{
    unsigned char *d = dst;
    while (n--)
        *d++ = (unsigned char)c;
    return dst;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    char *dp = dest;
    const char *sp = src;
    while (n--)
        *dp++ = *sp++;
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1 = s1;
    const unsigned char *p2 = s2;
    while (n--) {
        if (*p1 != *p2)
            return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

int atoi(const char *num)
{
    int value = 0;
    int sign = 1;

    if (*num == '-') {
        sign = -1;
        num++;
    }

    while (*num && is_digit(*num))
        value = value * 10 + (*num++ - '0');

    return value * sign;
}

u32 atoui(const char *num)
{
    u32 value = 0;

    if (num[0] == '0' && num[1] == 'x') {
        num += 2;
        while (*num && is_xdigit(*num))
            value = value * 16 + hexval(*num++);
    } else {
        while (*num && is_digit(*num))
            value = value * 10 + (*num++ - '0');
    }

    return value;
}

long atol(const char *num)
{
    long value = 0;
    int sign = 1;

    if (*num == '-') {
        sign = -1;
        num++;
    }

    if (num[0] == '0' && num[1] == 'x') {
        num += 2;
        while (*num && is_xdigit(*num))
            value = value * 16 + hexval(*num++);
    } else {
        while (*num && is_digit(*num))
            value = value * 10 + (*num++ - '0');
    }

    return value * sign;
}

unsigned long atoul(const char *num)
{
    unsigned long value = 0;

    if (num[0] == '0' && num[1] == 'x') {
        num += 2;
        while (*num && is_xdigit(*num))
            value = value * 16 + hexval(*num++);
    } else {
        while (*num && is_digit(*num))
            value = value * 10 + (*num++ - '0');
    }

    return value;
}

u64 atoull(const char *num)
{
    u64 value = 0;

    if (num[0] == '0' && num[1] == 'x') {
        num += 2;
        while (*num && is_xdigit(*num))
            value = value * 16 + hexval(*num++);
    } else {
        while (*num && is_digit(*num))
            value = value * 10 + (*num++ - '0');
    }

    return value;
}
