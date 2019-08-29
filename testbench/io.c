#include "io.h"

volatile uint32_t *UART_RX = (uint32_t *)0xBFD00000;
volatile uint32_t *UART_TX = (uint32_t *)0xBFD00004;
volatile uint32_t *UART_STAT = (uint32_t *)0xBFD00008;

volatile uint32_t *SPI_RESET = (uint32_t *)0xBFE10040;
volatile uint32_t *SPI_CONTROL = (uint32_t *)0xBFE10060;
volatile uint32_t *SPI_STATUS = (uint32_t *)0xBFE10064;
volatile uint8_t *SPI_TRANSMIT = (uint8_t *)0xBFE10068;
volatile uint8_t *SPI_RECEIVE = (uint8_t *)0xBFE1006C;
volatile uint32_t *SPI_SLAVESELECT = (uint32_t *)0xBFE10070;

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
    // echo
    putc(ch);
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
    if (temp < 10) {
      putc('0' + temp);
    } else if (temp < 16) {
      putc('A' + temp - 10);
    } else {
      putc('.');
    }
  }
}

uint32_t spi_status() { return *SPI_STATUS; }
uint32_t spi_control() { return *SPI_CONTROL; }

void spi_enable() {
  *SPI_RESET = 0xa;
  // trans inhibit
  // manual ss
  // rx fifo reset
  // tx fifo reset
  // master mode
  // enable
  *SPI_CONTROL = (1 << 8) | (1 << 7) | (1 << 6) | (1 << 5) | (1 << 2) | (1 << 1);
}

void spi_transfer(uint8_t *buffer, uint8_t len) {
  *SPI_SLAVESELECT = 0;
  for (int i = 0; i < len; i++) {
    *SPI_TRANSMIT = buffer[i];

    // disable trans inhibit
    *SPI_CONTROL = *SPI_CONTROL & ~(1 << 8);
    // wait for tx empty
    while ((*SPI_STATUS & (1 << 2)) == 0)
      ;

    // enable trans inhibit
    *SPI_CONTROL = *SPI_CONTROL | (1 << 8);
    
    // if rx not empty
    while ((*SPI_STATUS & (1 << 0)) == 0) {
      uint8_t data = *SPI_RECEIVE;
      buffer[i] = data;
    }
  }
  *SPI_SLAVESELECT = 1;
}

void spi_write_register(uint8_t addr, uint8_t data) {
  uint8_t writeBuffer[3];
  writeBuffer[0] = 0x40 | (addr >> 7);
  writeBuffer[1] = addr << 1;
  writeBuffer[2] = data;

  puts("SPI Write Register: ");
  puthex(addr);
  puts(" = ");
  puthex(data);
  puts("\r\n");

  spi_transfer(writeBuffer, 3);
}

uint8_t spi_read_register(uint8_t addr) {
  uint8_t readBuffer[3] = {0};
  readBuffer[0] = 0x60 | (addr >> 7);
  readBuffer[1] = addr << 1;
  readBuffer[2] = 0;

  spi_transfer(readBuffer, 3);
  puts("SPI Read Register: ");
  puthex(addr);
  puts(" = ");
  puthex(readBuffer[2]);
  puts("\r\n");

  return readBuffer[2];
}