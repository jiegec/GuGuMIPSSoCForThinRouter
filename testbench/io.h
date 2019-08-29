#include <stdint.h>

void putc(char ch);
uint8_t getc();
void gets(char *s);
void puts(char *s);
void puthex(uint32_t num);

uint32_t spi_status();
uint32_t spi_control();
void spi_enable();
void spi_write_register(uint8_t addr, uint8_t data);
uint8_t spi_read_register(uint8_t addr);

void fifo_poll_packet(uint32_t *buffer);