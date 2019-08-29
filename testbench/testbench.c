#include "io.h"
char buffer[1024];

int strequ(char *a, char *b) {
  while (*a && *b) {
    if (*a++ != *b++) {
      return 0;
    }
  }
  return *a == *b;
}

__attribute((section(".text.init")))
void main() {
  puts("ThinRouter TestBench\r\n");
  while (1) {
    puts(">> ");
    gets(buffer);
    puts("\r\n");
    if (strequ(buffer, "spi")) {
      puts("SPI status: ");
      puthex(spi_status());
      puts("\r\n");
    } else {
      puts("Nothing to do\r\n");
    }
  };
}