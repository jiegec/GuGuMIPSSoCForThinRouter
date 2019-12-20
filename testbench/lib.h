#include <stdint.h>
uint32_t bswap32(uint32_t arg);

uint16_t bswap16(uint16_t arg);

uint32_t htonl(uint32_t arg);

void memset(volatile void *buffer, char data, uint32_t count);

void memcpy(volatile void *to, volatile void *from, uint32_t count);

void memmove(volatile void *to, volatile void *from, uint32_t count);

int memcmp(volatile void *to, volatile void *from, uint32_t count);

void qsort(void *array, uint32_t nitems, uint32_t size,
           int (*cmp)(const void *, const void *));
