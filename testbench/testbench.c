#include <stdint.h>

volatile uint32_t *UART_RX = (uint32_t *)0xBFD00000;
volatile uint32_t *UART_TX = (uint32_t *)0xBFD00004;
volatile uint32_t *UART_STAT = (uint32_t *)0xBFD00008;

char buffer[1024];

void putc(char ch) {
  while (*UART_STAT & 0x8)
    ;
  *UART_TX = ch;
}

uint8_t getc() {
  while (!(*UART_STAT & 0x1))
    ;
  return (uint8_t)*UART_RX;
}

void gets(char *s) {
  while (1) {
    char ch = getc();
    if (ch == '\r' || ch == '\n') {
      break;
    }
    *s++ = ch;
  };
  *s++ = 0;
}

void puts(char *s) {
  while (*s) {
    putc(*s++);
  }
}

void puthex(uint32_t num) {
  int i, temp;
  for (i = 7; i >= 0; i--) {
    temp = (num >> (i * 4)) & 0xF;
    if (temp <= 10) {
      putc('0' + temp);
    } else if (temp < 16) {
      putc('A' + temp - 10);
    } else {
      putc('.');
    }
  }
}

__attribute((section(".text.init")))
void main() {
  puts("ThinRouter TestBench\r\n");
  while (1) {
    puts(">> ");
    gets(buffer);
    puts("\r\n");
    puts("Nothing to do\r\n");
  };
}