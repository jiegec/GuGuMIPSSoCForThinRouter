#include <stdint.h>

void putc(char ch);
uint8_t getc();
void gets(char *s);
void puts(char *s);
void puthex(uint32_t num);

uint32_t spi_status();