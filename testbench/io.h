#include <stdint.h>

void putc(char ch);
uint8_t getc();
void gets(char *s);
void puts(char *s);
void puthex(uint32_t num);
void puthex_u8(uint8_t num);
void putdec(uint32_t num);

static uint32_t htonl(uint32_t arg) {
  uint32_t ans = 0;
  ans |= (arg & 0xFF) << 24;
  ans |= (arg & 0xFF00) << 8;
  ans |= (arg & 0xFF0000) >> 8;
  ans |= (arg & 0xFF000000) >> 24;
  return ans;
}