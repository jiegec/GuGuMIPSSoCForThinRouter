#include "io.h"
#include "router_hal.h"
#include "xil_printf.h"

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

in_addr_t if_addrs[N_IFACE_ON_BOARD] = {0x0a000001, 0x0a000101, 0x0a000201,
                                        0x0a000301};

extern uint32_t _bss_start;
extern uint32_t _bss_end;

__attribute((section(".text.init"))) void main() {
  uint32_t *ptr = &_bss_start;
  uint32_t *end = &_bss_end;
  while (ptr != end) {
    *ptr++ = 0;
  }

  puts("ThinRouter TestBench\r\n");
  while (1) {
    puts(">> ");
    gets(buffer);
    puts("\r\n");
    if (strequ(buffer, "setup")) {
      puts("IP: ");
      gets(buffer);
      int ip = buffer[0] - '0';
      for (uint32_t i = 0; i < 4; i++) {
        if_addrs[i] = (i << 8) + 0x0a000000 + ip;
        xil_printf("IP %d: 10.0.%d.%d\n", i, i, ip);
      }
      HAL_Init(1, if_addrs);

      // confirms that register written is correct
      //if (spi_read_register(84) != 5) {
        //puts("Warning: SPI might not working properly");
      //}
    } else if (strequ(buffer, "echo")) {
      while (1) {
        macaddr_t src_mac;
        macaddr_t dst_mac;
        int if_index;
        int res = HAL_ReceiveIPPacket((1 << N_IFACE_ON_BOARD) - 1,
                                      (uint8_t *)packet, sizeof(packet),
                                      src_mac, dst_mac, -1, &if_index);
        xil_printf("res %d if_index %d\n", res, if_index);
        xil_printf("from ");
        for (int i = 0; i < 6; i++) {
          puthex_u8(src_mac[i]);
        }
        xil_printf(" to ");
        for (int i = 0; i < 6; i++) {
          puthex_u8(dst_mac[i]);
        }
        xil_printf("\n");
        for (int i = 0; i < res; i++) {
          puthex_u8(((uint8_t *)packet)[i]);
        }
        xil_printf("\n");
        //HAL_SendIPPacket(if_index, (uint8_t *)packet, res, src_mac);
      }
    } else if (strequ(buffer, "forward")) {
      while (1) {
        macaddr_t src_mac;
        macaddr_t dst_mac;
        int if_index;
        int res = HAL_ReceiveIPPacket((1 << N_IFACE_ON_BOARD) - 1,
                                      (uint8_t *)packet, sizeof(packet),
                                      src_mac, dst_mac, -1, &if_index);
        xil_printf("res %d if_index %d\n", res, if_index);
        xil_printf("from ");
        for (int i = 0; i < 6; i++) {
          puthex_u8(src_mac[i]);
        }
        xil_printf(" to ");
        for (int i = 0; i < 6; i++) {
          puthex_u8(dst_mac[i]);
        }
        xil_printf("\n");
        // static forwarding
        if (HAL_ArpGetMacAddress(1 - if_index,
                                 0x0200000a + ((1 - if_index) << 16),
                                 dst_mac) == 0) {
          HAL_SendIPPacket(1 - if_index, (uint8_t *)packet, res, dst_mac);
        } else {
          xil_printf("ARP not found for IP %x at port %d\n",
                     0x0200000a + ((1 - if_index) << 16), 1 - if_index);
        }
      }
    } else if (strequ(buffer, "broadcast")) {
      uint32_t ticks = HAL_GetTicks();
      int if_index = 0;
      while (1) {
        if ((uint32_t)HAL_GetTicks() - ticks < 1000) {
          continue;
        }
        ticks = HAL_GetTicks();
        if_index = (if_index + 1) % 4;
        int length = 64;
        for (int i = 0; i < length / 4; i++) {
          packet[i] = HAL_GetTicks();
        }

        macaddr_t dst_mac = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        HAL_SendIPPacket(if_index, (uint8_t *)packet, length, dst_mac);
        xil_printf("Sending to if %d ticks %d\n", if_index, ticks);
      }
    } else if (strequ(buffer, "rip")) {
      // Init
      HAL_Init(1, if_addrs);
      while (1) {
        macaddr_t src_mac;
        macaddr_t dst_mac;
        int if_index;
        int res = HAL_ReceiveIPPacket((1 << N_IFACE_ON_BOARD) - 1,
                                      (uint8_t *)packet, sizeof(packet),
                                      src_mac, dst_mac, -1, &if_index);
        xil_printf("res %d if_index %d\n", res, if_index);
        xil_printf("from ");
        for (int i = 0; i < 6; i++) {
          puthex_u8(src_mac[i]);
        }
        xil_printf(" to ");
        for (int i = 0; i < 6; i++) {
          puthex_u8(dst_mac[i]);
        }
        xil_printf("\n");
        for (int i = 0; i < res; i++) {
          puthex_u8(((uint8_t *)packet)[i]);
        }
        xil_printf("\n");
        // TODO
      }
    } else {
      puts("Nothing to do\r\n");
    }
  };
}