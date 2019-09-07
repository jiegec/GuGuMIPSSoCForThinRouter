#include "io.h"

volatile uint32_t *UART_RX = (uint32_t *)0xBFD00000;
volatile uint32_t *UART_TX = (uint32_t *)0xBFD00004;
volatile uint32_t *UART_STAT = (uint32_t *)0xBFD00008;

volatile uint32_t *SPI_RESET = (uint32_t *)0xBFE00040;
volatile uint32_t *SPI_CONTROL = (uint32_t *)0xBFE00060;
volatile uint32_t *SPI_STATUS = (uint32_t *)0xBFE00064;
volatile uint8_t *SPI_TRANSMIT = (uint8_t *)0xBFE00068;
volatile uint8_t *SPI_RECEIVE = (uint8_t *)0xBFE0006C;
volatile uint32_t *SPI_SLAVESELECT = (uint32_t *)0xBFE00070;

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
  *SPI_CONTROL =
      (1 << 8) | (1 << 7) | (1 << 6) | (1 << 5) | (1 << 2) | (1 << 1);
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

void eth_poll_packet(uint32_t *buffer) {
  puts("Polling for packet\r\n");

  puts("S2MM Status: ");
  puthex(*DMA_S2MM_DMASR);
  puts("\r\n");

  for (int i = 0; i < BD_COUNT; i++) {
    volatile struct DMADesc *current =
        (struct DMADesc *)((uint32_t)&rxBdSpace[i] + UNCACHED_MEMORY_OFFSET);
    if (i != BD_COUNT - 1) {
      current->nextDescLo =
          ((uint32_t)&rxBdSpace[i + 1]) - PHYSICAL_MEMORY_OFFSET;
      current->nextDescHi = 0;
    } else {
      current->nextDescLo = ((uint32_t)&rxBdSpace[0]) - PHYSICAL_MEMORY_OFFSET;
      current->nextDescHi = 0;
    }
    current->bufferAddrLo = ((uint32_t)&rxBufSpace[i]) - PHYSICAL_MEMORY_OFFSET;
    current->bufferAddrHi = 0;
    current->reserved[0] = current->reserved[1] = 0;
    current->control = BUFFER_SIZE;
    current->status = 0;
    current->application[0] = 0;
    current->application[1] = 0;
    current->application[2] = 0;
    current->application[3] = 0;
    current->application[4] = 0;
    current->padding[0] = 0;
    current->padding[1] = 0;
    current->padding[2] = 0;
  }

  *DMA_S2MM_CURDESC = ((uint32_t)&rxBdSpace[0]) - PHYSICAL_MEMORY_OFFSET;
  *DMA_S2MM_CURDESC_HI = 0;

  *DMA_S2MM_DMACR = 1 << 0;

  *DMA_S2MM_TAILDESC =
      ((uint32_t)&rxBdSpace[BD_COUNT - 1]) - PHYSICAL_MEMORY_OFFSET;
  *DMA_S2MM_TAILDESC_HI = 0;

  int index = 0;
  while (1) {
    volatile struct DMADesc *current =
        (struct DMADesc *)((uint32_t)&rxBdSpace[index] +
                           UNCACHED_MEMORY_OFFSET);
    if (current->status) {
      puts("Desc is filled with status ");
      puthex(current->status);
      puts("\r\n");

      uint32_t len = (uint16_t)current->status;
      uint8_t *buffer =
          (uint8_t *)((uint32_t)&rxBufSpace[index][0] + UNCACHED_MEMORY_OFFSET);
      for (int i = 0; i < len; i++) {
        puthex_u8(buffer[i]);
      }
      puts("\r\n");

      current->reserved[0] = current->reserved[1] = 0;
      current->control = BUFFER_SIZE;
      current->status = 0;
      current->application[0] = 0;
      current->application[1] = 0;
      current->application[2] = 0;
      current->application[3] = 0;
      current->application[4] = 0;
      current->padding[0] = 0;
      current->padding[1] = 0;
      current->padding[2] = 0;

      *DMA_S2MM_TAILDESC =
          ((uint32_t)&rxBdSpace[index]) - PHYSICAL_MEMORY_OFFSET;
      *DMA_S2MM_TAILDESC_HI = 0;

      index++;
      if (index == BD_COUNT) {
        index = 0;
      }
    }
  }
  /*
  // reset
  *FIFO_RDFR = 0xA5;
  while (1) {
    if ((*FIFO_ISR & (1 << 30)) == 1) {
      // clear
      *FIFO_ISR = *FIFO_ISR;
      // reset
      *FIFO_RDFR = 0xA5;
      puts("FIFO overrun\r\n");
    }
    if (*FIFO_RDFO) {
      uint32_t real_len = *FIFO_RLR;
      uint32_t len = real_len / 4;
      puts("Got packet of length ");
      putdec(real_len);
      puts("\r\n");
      puts("Data: ");
      for (uint32_t i = 0; i < len; i++) {
        uint32_t data = *FIFO_RDFD;
        buffer[i] = data;
        puthex_be(data);
      }
      puts("\r\n");
    }
  }
  */
}