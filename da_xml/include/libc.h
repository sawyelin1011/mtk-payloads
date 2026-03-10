/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#ifndef DA_XML_LIBC_H
#define DA_XML_LIBC_H

#include <stdint.h>
#include <stddef.h>

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long u64;


#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ROUNDUP(a, b) (((a) + ((b)-1)) & ~((b)-1))
#define ROUNDDOWN(a, b) ((a) & ~((b)-1))

#define ALIGN(a, b) ROUNDUP(a, b)
#define IS_ALIGNED(a, b) (!(((uintptr_t)(a)) & (((uintptr_t)(b))-1)))

size_t strlen(const char *str);
char *strcpy(char *to, const char *from);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, u32 n);
char *strstr(const char *s1, const char *s2);
int split(char *src, char **dest, int max, char sep);

void *memset(void*  dst, int c, u32 n);
void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);

int atoi(const char *num);
u32 atoui(const char *num);
long atol(const char *num);
unsigned long atoul(const char *num);
u64 atoull(const char *num);


#endif //DA_XML_LIBC_H
