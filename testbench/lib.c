#include <stdint.h>
void memset(void *buffer, char data, uint32_t count) {
  char *p = buffer;
  for (int i = 0; i < count; i++) {
    p[i] = data;
  }
}

void memcpy(void *to, void *from, uint32_t count) {
  char *t = to;
  char *f = from;
  for (int i = 0; i < count; i++) {
    t[i] = f[i];
  }
}

void memmove(void *to, void *from, uint32_t count) {
  uint32_t to_addr = (uint32_t)to;
  uint32_t from_addr = (uint32_t)from;
  if (to_addr < from_addr) {
    memcpy(to, from, count);
  } else if (to_addr > from_addr) {
    char *t = to;
    char *f = from;
    for (int i = count - 1; i >= 0; i--) {
        t[i] = f[i];
    }
  }
}

int memcmp(void *to, void *from, uint32_t count) {
  char *t = to;
  char *f = from;
  for (int i = 0; i < count; i++) {
      if (t[i] != f[i]) {
        return t[i] - f[i];
      }
  }
  return 0;
}

uint32_t bswap32(uint32_t arg) {
  uint32_t ans = 0;
  ans |= (arg & 0xFF) << 24;
  ans |= (arg & 0xFF00) << 8;
  ans |= (arg & 0xFF0000) >> 8;
  ans |= (arg & 0xFF000000) >> 24;
  return ans;
}

uint16_t bswap16(uint16_t arg) {
  uint16_t ans = 0;
  ans |= (arg & 0xFF) << 8;
  ans |= (arg & 0xFF00) >> 8;
  return ans;
}

uint32_t htonl(uint32_t arg) { return bswap32(arg); }