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

void XAxiDma_BdRingAlloc(XAxiDma_BdRing *bd, int count, XAxiDma_Bd **res) {}

void XAxiDma_BdRingUnAlloc(XAxiDma_BdRing *bd, int count, XAxiDma_Bd *res) {}

void XAxiDma_BdRingFree(XAxiDma_BdRing *bd, int count, XAxiDma_Bd *res) {}

void XAxiDma_BdRingToHw(XAxiDma_BdRing *bd, int count, XAxiDma_Bd *res) {}

int XAxiDma_BdRingFromHw(XAxiDma_BdRing *bd, int count, XAxiDma_Bd **res) {}

#define XPAR_AXIDMA_0_DEVICE_ID 0
XAxiDma_Config *XAxiDma_LookupConfig(int id) { return NULL; }
void XAxiDma_CfgInitialize(XAxiDma *dma, XAxiDma_Config *cfg) {}

XAxiDma_BdRing *XAxiDma_GetRxRing(XAxiDma *dma) { return NULL; }
XAxiDma_BdRing *XAxiDma_GetTxRing(XAxiDma *dma) { return NULL; }

void memset(volatile void *buffer, char data, uint32_t count);

void memcpy(volatile void *to, volatile void *from, uint32_t count);

void memmove(volatile void *to, volatile void *from, uint32_t count);

int memcmp(volatile void *to, volatile void *from, uint32_t count);

#define UINTPTR uint32_t
void XAxiDma_BdRingCreate(XAxiDma_BdRing *ring, uint32_t space, uint32_t space2,
                          uint32_t align, uint32_t count) {}
XAxiDma_Bd *XAxiDma_BdRingNext(XAxiDma_BdRing *ring, XAxiDma_Bd *bd) {}
void XAxiDma_BdRingStart(XAxiDma_BdRing *bd) {}
void XAxiDma_BdClear(XAxiDma_Bd* bd) {}
void XAxiDma_BdSetCtrl(XAxiDma_Bd* bd, uint32_t flags) {}
#define XAXIDMA_BD_CTRL_TXSOF_MASK  (1 << 27)
#define XAXIDMA_BD_CTRL_TXEOF_MASK  (1 << 28)
#define XAXIDMA_BD_USR4_OFFSET 0
u32 XAxiDma_BdRead(XAxiDma_Bd *bd, uint32_t offset) {

}