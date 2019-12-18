#include "router_hal.h"
#include "io.h"
#include "structs.h"
#include "lib.h"
#include "xaxidma.h"
#include "xil_printf.h"

const int IP_OFFSET = 4 + 14;
const int ARP_LENGTH = 28;

int inited = 0;
int debugEnabled = 0;
in_addr_t interface_addrs[N_IFACE_ON_BOARD] = {0};
macaddr_t interface_mac = {2, 2, 3, 3, 0, 0};

#define BD_COUNT 128
#define BUFFER_SIZE 2048
#define PHYSICAL_MEMORY_OFFSET 0x80000000
#define UNCACHED_MEMORY_OFFSET 0x20000000

extern volatile uint32_t *DMA_S2MM_DMACR;
extern volatile uint32_t *DMA_S2MM_DMASR;
extern volatile uint32_t *DMA_S2MM_CURDESC;
extern volatile uint32_t *DMA_S2MM_CURDESC_HI;
extern volatile uint32_t *DMA_S2MM_TAILDESC;
extern volatile uint32_t *DMA_S2MM_TAILDESC_HI;

extern volatile uint32_t *DMA_MM2S_DMACR;
extern volatile uint32_t *DMA_MM2S_DMASR;
extern volatile uint32_t *DMA_MM2S_CURDESC;
extern volatile uint32_t *DMA_MM2S_CURDESC_HI;
extern volatile uint32_t *DMA_MM2S_TAILDESC;
extern volatile uint32_t *DMA_MM2S_TAILDESC_HI;

volatile uint32_t *ROUTER_RESET_N = (uint32_t *)0xBFA00000;

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

int rxIndex;
int txIndex;
volatile struct DMADesc rxBdSpace[BD_COUNT]
    __attribute__((aligned(sizeof(struct DMADesc))));
uint8_t rxBufSpace[BD_COUNT][BUFFER_SIZE];
volatile struct DMADesc txBdSpace[BD_COUNT]
    __attribute__((aligned(sizeof(struct DMADesc))));
uint8_t txBufSpace[BD_COUNT][BUFFER_SIZE];

struct EthernetTaggedFrame {
  u8 dstMAC[6];
  u8 srcMAC[6];
  u16 vlanEtherType;
  u16 vlanID;
  u16 etherType;
  u8 data[1500];
};

#define ARP_TABLE_SIZE 16

// simple FIFO cache
struct ArpTableEntry {
  int if_index;
  macaddr_t mac;
  in_addr_t ip;
} arpTable[ARP_TABLE_SIZE];

void PutBackBd(XAxiDma_Bd *bd) {
  volatile struct DMADesc *current =
      (struct DMADesc *)((uint32_t)&rxBdSpace[rxIndex] +
                         UNCACHED_MEMORY_OFFSET);
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

  *DMA_S2MM_TAILDESC = ((uint32_t)&rxBdSpace[rxIndex]) - PHYSICAL_MEMORY_OFFSET;
  *DMA_S2MM_TAILDESC_HI = 0;

  rxIndex++;
  if (rxIndex == BD_COUNT) {
    rxIndex = 0;
  }
}

void WaitTxBdAvailable() {
  volatile struct DMADesc *current =
      (struct DMADesc *)((uint32_t)&txBdSpace[txIndex] +
                         UNCACHED_MEMORY_OFFSET);
  while (current->control != 0 && current->status == 0)
    ;
}

int HAL_Init(int debug, in_addr_t if_addrs[N_IFACE_ON_BOARD]) {
  XAxiDma_Bd *bd;
  if (inited) {
    return 0;
  }
  debugEnabled = debug;

  if (debugEnabled) {
    xil_printf("HAL_Init: Init vlan %x\n\r", rxBdSpace);
  }
  /*
  // P1-P4 Tag Removal
  SpiWriteRegister(16, 2);
  SpiWriteRegister(32, 2);
  SpiWriteRegister(48, 2);
  SpiWriteRegister(64, 2);
  // P5 Tag Insertion
  SpiWriteRegister(80, 4);
  // P1-P5 PVID
  SpiWriteRegister(20, 1);
  SpiWriteRegister(36, 2);
  SpiWriteRegister(52, 3);
  SpiWriteRegister(68, 4);
  SpiWriteRegister(84, 5);
  // P1-P4 membership
  SpiWriteRegister(17, (1 << 4) | (1 << 0));
  SpiWriteRegister(33, (1 << 4) | (1 << 1));
  SpiWriteRegister(49, (1 << 4) | (1 << 2));
  SpiWriteRegister(65, (1 << 4) | (1 << 3));
  */

  if (debugEnabled) {
    xil_printf("HAL_Init: Init rings @ %x\r\n", rxBdSpace);
  }

  memset(rxBdSpace, 0, sizeof(rxBdSpace));
  memset(txBdSpace, 0, sizeof(txBdSpace));

  if (debugEnabled) {
    xil_printf("HAL_Init: Enable Ethernet MAC\r\n");
  }

  if (debugEnabled) {
    xil_printf("HAL_Init: Add buffer to rings\r\n");
  }

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

  // reset
  *ROUTER_RESET_N = 0;
  *DMA_S2MM_DMACR = 1 << 2;
  while (*DMA_S2MM_DMACR & (1 << 2))
    ;
  *DMA_MM2S_DMACR = 1 << 2;
  while (*DMA_MM2S_DMACR & (1 << 2))
    ;

  *DMA_S2MM_CURDESC = ((uint32_t)&rxBdSpace[0]) - PHYSICAL_MEMORY_OFFSET;
  *DMA_S2MM_CURDESC_HI = 0;

  *DMA_S2MM_DMACR = 1 << 0;

  *DMA_S2MM_TAILDESC =
      ((uint32_t)&rxBdSpace[BD_COUNT - 1]) - PHYSICAL_MEMORY_OFFSET;
  *DMA_S2MM_TAILDESC_HI = 0;
  rxIndex = 0;

  for (int i = 0; i < BD_COUNT; i++) {
    volatile struct DMADesc *current =
        (struct DMADesc *)((uint32_t)&txBdSpace[i] + UNCACHED_MEMORY_OFFSET);
    if (i != BD_COUNT - 1) {
      current->nextDescLo =
          ((uint32_t)&txBdSpace[i + 1]) - PHYSICAL_MEMORY_OFFSET;
      current->nextDescHi = 0;
    } else {
      current->nextDescLo = ((uint32_t)&txBdSpace[0]) - PHYSICAL_MEMORY_OFFSET;
      current->nextDescHi = 0;
    }
    current->bufferAddrLo = ((uint32_t)&txBufSpace[i]) - PHYSICAL_MEMORY_OFFSET;
    current->bufferAddrHi = 0;
    current->reserved[0] = current->reserved[1] = 0;
    current->control = 0;
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

  *DMA_MM2S_CURDESC = ((uint32_t)&txBdSpace[0]) - PHYSICAL_MEMORY_OFFSET;
  *DMA_MM2S_CURDESC_HI = 0;

  *DMA_MM2S_DMACR = 1 << 0;

  // start router ip
  *ROUTER_RESET_N = 1;

  txIndex = 0;

  memcpy(interface_addrs, if_addrs, sizeof(interface_addrs));
  memset(arpTable, 0, sizeof(arpTable));

  inited = 1;
  return 0;
}

uint64_t HAL_GetTicks() {
  static uint32_t cp0_count_hi = 0;
  static uint32_t last_cp0_count_hi = 0;
  uint32_t cp0_count;
  asm volatile("mfc0 %0, $9, 0;" : "=r"(cp0_count));
  if (cp0_count & 0xf0000000 != last_cp0_count_hi) {
    last_cp0_count_hi = cp0_count & 0xf0000000;
    cp0_count_hi++;
  }
  return cp0_count_hi * 85900 + cp0_count / 50000;
}

int HAL_ArpGetMacAddress(int if_index, in_addr_t ip, macaddr_t o_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }

  for (int i = 0; i < ARP_TABLE_SIZE; i++) {
    if (arpTable[i].if_index == if_index && arpTable[i].ip == ip) {
      memcpy(o_mac, arpTable[i].mac, sizeof(macaddr_t));
      return 0;
    }
  }

  if (debugEnabled) {
    xil_printf(
        "HAL_ArpGetMacAddress: asking for ip address with arp request\r\n");
  }
  // request
  WaitTxBdAvailable();
  volatile struct DMADesc *current =
      (struct DMADesc *)((uint32_t)&txBdSpace[txIndex] +
                         UNCACHED_MEMORY_OFFSET);
  // skip nextDesc fields
  memset(((uint8_t *)current + 8), 0, sizeof(struct DMADesc) - 8);
  current->bufferAddrLo =
      (uint32_t)&txBufSpace[txIndex] - PHYSICAL_MEMORY_OFFSET;
  current->control = (uint16_t)(ARP_LENGTH + IP_OFFSET);
  current->control = current->control | XAXIDMA_BD_CTRL_TXSOF_MASK |
                     XAXIDMA_BD_CTRL_TXEOF_MASK;

  u8 *buffer = (u8 *)((uint32_t)&txBufSpace[txIndex] + UNCACHED_MEMORY_OFFSET);

  // dst mac
  for (int i = 0; i < 6; i++) {
    buffer[i] = 0xff;
  }
  // src mac
  memcpy(&buffer[6], interface_mac, sizeof(macaddr_t));
  // VLAN
  buffer[12] = 0x81;
  buffer[13] = 0x00;
  // PID
  buffer[14] = 0x00;
  buffer[15] = if_index + 1;
  // ARP
  buffer[16] = 0x08;
  buffer[17] = 0x06;
  // hardware type
  buffer[18] = 0x00;
  buffer[19] = 0x01;
  // protocol type
  buffer[20] = 0x08;
  buffer[21] = 0x00;
  // hardware size
  buffer[22] = 0x06;
  // protocol size
  buffer[23] = 0x04;
  // opcode
  buffer[24] = 0x00;
  buffer[25] = 0x01;
  // sender
  memcpy(&buffer[26], interface_mac, sizeof(macaddr_t));
  in_addr_t sender_ip = htonl(interface_addrs[if_index]);
  memcpy(&buffer[32], &sender_ip, sizeof(in_addr_t));
  // target
  memset(&buffer[36], 0, sizeof(macaddr_t));
  memcpy(&buffer[42], &ip, sizeof(in_addr_t));

  *DMA_MM2S_TAILDESC = ((uint32_t)&txBdSpace[txIndex]) - PHYSICAL_MEMORY_OFFSET;
  txIndex++;
  if (txIndex == BD_COUNT) {
    txIndex = 0;
  }
  return HAL_ERR_IP_NOT_EXIST;
}

int HAL_GetInterfaceMacAddress(int if_index, macaddr_t o_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_IFACE_NOT_EXIST;
  }

  memcpy(o_mac, interface_mac, sizeof(macaddr_t));
  return 0;
}

int HAL_ReceiveIPPacket(int if_index_mask, uint8_t *buffer, size_t length,
                        macaddr_t src_mac, macaddr_t dst_mac, int64_t timeout,
                        int *if_index) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if ((if_index_mask & ((1 << N_IFACE_ON_BOARD) - 1)) == 0 ||
      (timeout < 0 && timeout != -1)) {
    return HAL_ERR_INVALID_PARAMETER;
  }
  if (if_index_mask != ((1 << N_IFACE_ON_BOARD) - 1)) {
    return HAL_ERR_NOT_SUPPORTED;
  }
  XAxiDma_Bd *bd;
  uint64_t begin = HAL_GetTicks();
  uint64_t current_time = 0;
  while ((current_time = HAL_GetTicks()) < begin + timeout || timeout == -1) {
    volatile struct DMADesc *current =
        (struct DMADesc *)((uint32_t)&rxBdSpace[rxIndex] +
                           UNCACHED_MEMORY_OFFSET);
    if (current->status) {
      // See AXI Ethernet Table 3-15
      u32 length = (uint16_t)current->status;
      uint8_t *data = (uint8_t *)((uint32_t)&rxBufSpace[rxIndex][0] +
                                  UNCACHED_MEMORY_OFFSET);
      // skip port number
      data = &data[1];
      if (data && length >= IP_OFFSET && data[12] == 0x81 && data[13] == 0x00 &&
          data[16] == 0x08 && data[17] == 0x00) {
        // IPv4
        memcpy(dst_mac, data, sizeof(macaddr_t));
        memcpy(src_mac, &data[6], sizeof(macaddr_t));
        // Vlan ID 1-4
        *if_index = data[15] - 1;

        size_t ip_len = length - IP_OFFSET;
        size_t real_length = length > ip_len ? ip_len : length;
        memcpy(buffer, &data[IP_OFFSET], real_length);

        PutBackBd(bd);
        return real_length;
      } else if (data && length >= IP_OFFSET + ARP_LENGTH && data[12] == 0x81 &&
                 data[13] == 0x00 && data[16] == 0x08 && data[17] == 0x06) {
        // ARP
        macaddr_t mac;
        memcpy(mac, &data[26], sizeof(macaddr_t));
        in_addr_t ip;
        memcpy(&ip, &data[32], sizeof(in_addr_t));
        u32 vlan = data[15] - 1;

        // update ARP Table
        int insert = 1;
        for (int i = 0; i < ARP_TABLE_SIZE; i++) {
          if (arpTable[i].if_index == vlan &&
              memcmp(arpTable[i].mac, mac, sizeof(macaddr_t)) == 0) {
            arpTable[i].ip = ip;
            insert = 0;
            break;
          }
        }

        if (insert) {
          memmove(&arpTable[1], arpTable,
                  (ARP_TABLE_SIZE - 1) * sizeof(struct ArpTableEntry));
          arpTable[0].if_index = vlan;
          memcpy(arpTable[0].mac, mac, sizeof(macaddr_t));
          arpTable[0].ip = ip;
        }
        if (debugEnabled) {
          xil_printf(
              "HAL_ReceiveIPPacket: learned ARP from %d.%d.%d.%d vlan %d\r\n",
              ip & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF, ip >> 24, vlan);
        }

        in_addr_t dst_ip;
        memcpy(&dst_ip, &data[42], sizeof(in_addr_t));
        if (vlan < N_IFACE_ON_BOARD && htonl(dst_ip) == interface_addrs[vlan]) {
          // reply
          if (debugEnabled) {
            xil_printf("HAL_ReceiveIPPacket: reply ARP to %d.%d.%d.%d\r\n",
                       ip & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF,
                       ip >> 24);
          }
          WaitTxBdAvailable();
          volatile struct DMADesc *current =
              (struct DMADesc *)((uint32_t)&txBdSpace[txIndex] +
                                 UNCACHED_MEMORY_OFFSET);
          // skip nextDesc fields
          memset(((uint8_t *)current + 8), 0, sizeof(struct DMADesc) - 8);
          current->bufferAddrLo =
              (uint32_t)&txBufSpace[txIndex] - PHYSICAL_MEMORY_OFFSET;
          current->control = (uint16_t)(IP_OFFSET + ARP_LENGTH + 1);
          current->control = current->control | XAXIDMA_BD_CTRL_TXSOF_MASK |
                             XAXIDMA_BD_CTRL_TXEOF_MASK;

          u8 *buffer =
              (u8 *)((uint32_t)&txBufSpace[txIndex] + UNCACHED_MEMORY_OFFSET);
          buffer[0] = 0; // router port
          buffer = &buffer[1];
          memcpy(buffer, &data[6], sizeof(macaddr_t));
          memcpy(&buffer[6], interface_mac, sizeof(macaddr_t));
          // VLAN
          buffer[12] = 0x81;
          buffer[13] = 0x00;
          // PID
          buffer[14] = 0x00;
          buffer[15] = vlan + 1;
          // ARP
          buffer[16] = 0x08;
          buffer[17] = 0x06;
          // hardware type
          buffer[18] = 0x00;
          buffer[19] = 0x01;
          // protocol type
          buffer[20] = 0x08;
          buffer[21] = 0x00;
          // hardware size
          buffer[22] = 0x06;
          // protocol size
          buffer[23] = 0x04;
          // opcode
          buffer[24] = 0x00;
          buffer[25] = 0x02;
          // sender
          memcpy(&buffer[26], interface_mac, sizeof(macaddr_t));
          memcpy(&buffer[32], &dst_ip, sizeof(in_addr_t));
          // target
          memcpy(&buffer[36], &data[26], sizeof(macaddr_t));
          memcpy(&buffer[42], &data[32], sizeof(in_addr_t));

          *DMA_MM2S_TAILDESC =
              ((uint32_t)&txBdSpace[txIndex]) - PHYSICAL_MEMORY_OFFSET;
          txIndex++;
          if (txIndex == BD_COUNT) {
            txIndex = 0;
          }

          if (debugEnabled) {
            xil_printf("HAL_ReceiveIPPacket: replied ARP to %d.%d.%d.%d\r\n",
                       ip & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF,
                       ip >> 24);
          }
        }
      } else {
        if (debugEnabled) {
          xil_printf(
              "HAL_ReceiveIPPacket: ignore unrecognized packet at time %d\r\n",
              (uint32_t)HAL_GetTicks());
        }
      }
      PutBackBd(bd);
    }
  }
  return 0;
}

int HAL_SendIPPacket(int if_index, uint8_t *buffer, size_t length,
                     macaddr_t dst_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }
  WaitTxBdAvailable();
  volatile struct DMADesc *current =
      (struct DMADesc *)((uint32_t)&txBdSpace[txIndex] +
                         UNCACHED_MEMORY_OFFSET);
  // skip nextDesc fields
  memset(((uint8_t *)current + 8), 0, sizeof(struct DMADesc) - 8);
  current->bufferAddrLo =
      (uint32_t)&txBufSpace[txIndex] - PHYSICAL_MEMORY_OFFSET;
  current->control = (uint16_t)(length + IP_OFFSET + 1);
  current->control = current->control | XAXIDMA_BD_CTRL_TXSOF_MASK |
                     XAXIDMA_BD_CTRL_TXEOF_MASK;

  u8 *data = (u8 *)((uint32_t)&txBufSpace[txIndex] + UNCACHED_MEMORY_OFFSET);
  // skip port
  data[0] = 0;
  data = &data[1];
  memcpy(data, dst_mac, sizeof(macaddr_t));
  memcpy(&data[6], interface_mac, sizeof(macaddr_t));
  // VLAN
  data[12] = 0x81;
  data[13] = 0x00;
  // PID
  data[14] = 0x00;
  data[15] = if_index + 1;
  // IPv4
  data[16] = 0x08;
  data[17] = 0x00;
  memcpy(&data[IP_OFFSET], buffer, length);

  *DMA_MM2S_TAILDESC = ((uint32_t)&txBdSpace[txIndex]) - PHYSICAL_MEMORY_OFFSET;
  txIndex++;
  if (txIndex == BD_COUNT) {
    txIndex = 0;
  }
  return 0;
}
