#include "io.h"
char buffer[1024];
uint32_t packet[1024];

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
      puts("SPI control: ");
      puthex(spi_control());
      puts("\r\n");
    } else if (strequ(buffer, "setup")) {
      spi_enable();
      // P1-P4 Tag Removal
      spi_write_register(16, 2);
      spi_write_register(32, 2);
      spi_write_register(48, 2);
      spi_write_register(64, 2);
      // P5 Tag Removal
      spi_write_register(80, 4);
      // P1-P5 PVID
      spi_write_register(20, 1);
      spi_write_register(36, 2);
      spi_write_register(52, 3);
      spi_write_register(68, 4);
      spi_write_register(84, 5);

      // confirms that register written is correct
      if (spi_read_register(84) != 5) {
        puts("Warning: SPI might not working properly");
      }
    } else if (strequ(buffer, "poll")) {
      eth_poll_packet(packet);
    } else {
      puts("Nothing to do\r\n");
    }
  };
}