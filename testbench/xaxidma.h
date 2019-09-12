#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef struct XAxiDma_Config {
} XAxiDma_Config;
typedef struct XAxiDma {
} XAxiDma;
typedef struct XAxiDma_BdRing {
  uint32_t MaxTransferLen;
} XAxiDma_BdRing;
typedef struct XAxiDma_Bd {
  uint32_t nextDescLo;
  uint32_t nextDescHi;
  uint32_t bufferAddrLo;
  uint32_t bufferAddrHi;
  uint32_t reserved[2];
  uint32_t control;
  uint32_t status;
  uint32_t application[5];
  uint32_t padding[3];
} XAxiDma_Bd;

#define XAxiDma_BdRingMemCalc(align, count) ((count)*0x40)
#define XAXIDMA_BD_MINIMUM_ALIGNMENT 0x40

#define PHYSICAL_MEMORY_OFFSET 0x80000000
#define UNCACHED_MEMORY_OFFSET 0x20000000

u32 XAxiDma_BdGetBufAddr(XAxiDma_Bd *bd) {
  return bd->bufferAddrLo + PHYSICAL_MEMORY_OFFSET + UNCACHED_MEMORY_OFFSET;
}

void XAxiDma_BdSetBufAddr(XAxiDma_Bd *bd, u32 addr) {
  bd->bufferAddrLo = addr - UNCACHED_MEMORY_OFFSET - PHYSICAL_MEMORY_OFFSET;
}

u32 XAxiDma_BdGetLength(XAxiDma_Bd *bd, int mask) { return (u16)bd->control; }

void XAxiDma_BdSetLength(XAxiDma_Bd *bd, u32 length, int mask) {
  bd->control = length | ((u32)((u16)bd->control) << 16);
}

void memset(volatile void *buffer, char data, uint32_t count);

void memcpy(volatile void *to, volatile void *from, uint32_t count);

void memmove(volatile void *to, volatile void *from, uint32_t count);

int memcmp(volatile void *to, volatile void *from, uint32_t count);

#define UINTPTR uint32_t
#define XAXIDMA_BD_CTRL_TXEOF_MASK  (1 << 26)
#define XAXIDMA_BD_CTRL_TXSOF_MASK  (1 << 27)
#define XAXIDMA_BD_USR4_OFFSET 0