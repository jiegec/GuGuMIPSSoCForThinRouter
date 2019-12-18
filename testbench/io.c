#include "io.h"

volatile uint32_t *UART_RX = (uint32_t *)0xBFD00000;
volatile uint32_t *UART_TX = (uint32_t *)0xBFD00004;
volatile uint32_t *UART_STAT = (uint32_t *)0xBFD00008;

volatile uint32_t *FIFO_ISR = (uint32_t *)0xBFB00000;
volatile uint32_t *FIFO_RDFR = (uint32_t *)0xBFB00018;
volatile uint32_t *FIFO_RDFO = (uint32_t *)0xBFB0001C;
volatile uint32_t *FIFO_RDFD = (uint32_t *)0xBFB00020;
volatile uint32_t *FIFO_RLR = (uint32_t *)0xBFB00024;

volatile uint32_t *DMA_S2MM_DMACR = (uint32_t *)0xBFF00030;
volatile uint32_t *DMA_S2MM_DMASR = (uint32_t *)0xBFF00034;
volatile uint32_t *DMA_S2MM_CURDESC = (uint32_t *)0xBFF00038;
volatile uint32_t *DMA_S2MM_CURDESC_HI = (uint32_t *)0xBFF0003C;
volatile uint32_t *DMA_S2MM_TAILDESC = (uint32_t *)0xBFF00040;
volatile uint32_t *DMA_S2MM_TAILDESC_HI = (uint32_t *)0xBFF00044;

volatile uint32_t *DMA_MM2S_DMACR = (uint32_t *)0xBFF00000;
volatile uint32_t *DMA_MM2S_DMASR = (uint32_t *)0xBFF00004;
volatile uint32_t *DMA_MM2S_CURDESC = (uint32_t *)0xBFF00008;
volatile uint32_t *DMA_MM2S_CURDESC_HI = (uint32_t *)0xBFF0000C;
volatile uint32_t *DMA_MM2S_TAILDESC = (uint32_t *)0xBFF00010;
volatile uint32_t *DMA_MM2S_TAILDESC_HI = (uint32_t *)0xBFF00014;

#define BD_COUNT 16
#define BUFFER_SIZE 2048
#define PHYSICAL_MEMORY_OFFSET 0x80000000
#define UNCACHED_MEMORY_OFFSET 0x20000000

struct DMADesc {
  uint32_t nextDescLo;
  uint32_t nextDescHi;
  uint32_t bufferAddrLo;
  uint32_t bufferAddrHi;
  uint32_t reserved[2];
  uint32_t control;
  uint32_t status;
  uint32_t application[5];
  uint32_t padding[3];
};

volatile struct DMADesc rxBdSpace[BD_COUNT]
    __attribute__((aligned(sizeof(struct DMADesc))));
uint8_t rxBufSpace[BD_COUNT][BUFFER_SIZE];
volatile struct DMADesc txBdSpace[BD_COUNT]
    __attribute__((aligned(sizeof(struct DMADesc))));
uint8_t txBufSpace[BD_COUNT][BUFFER_SIZE];

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

void putdec(uint32_t num) {
  if (num >= 10) {
    putdec(num / 10);
  }
  putc('0' + (num % 10));
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

void puthex_u8(uint8_t num) {
  int i, temp;
  for (i = 1; i >= 0; i--) {
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

void puthex_be(uint32_t num) {
  uint32_t le;
  le = (num & 0xFF) << 24;
  le |= (num & 0xFF00) << 8;
  le |= (num & 0xFF0000) >> 8;
  le |= (num & 0xFF000000) >> 24;
  puthex(le);
}