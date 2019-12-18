#include "io.h"
#include "lib.h"
#include "router_hal.h"
#include "structs.h"
#include "xil_printf.h"

char buffer[1024];
uint32_t packet[1024];
u32 packet_buffer[512];
struct Route routingTable[1024];
int routingTableSize = 0;
u8 ripMAC[6] = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x09};

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

u16 checksumAdd(u16 orig, u16 add) {
  u32 ans = orig;
  ans += add;
  ans = (ans >> 16) + (ans & 0xFFFF);
  ans = (ans >> 16) + (ans & 0xFFFF);
  return (u16)ans;
}

void fillIpChecksum(struct Ip *ip) {
  u16 *data = ((u16 *)ip);
  ip->headerChecksum = 0;
  u16 checksum = 0;
  for (int i = 0; i < 10; i++) {
    checksum = checksumAdd(checksum, data[i]);
  }
  ip->headerChecksum = ~checksum;
}

void sendRIPReponse() {
  // send RIP response
  for (u8 port = 0; port < 4; port++) {
    u8 portIP[] = {10, 0, port, 1};
    u8 ripIP[4] = {224, 0, 0, 9};
    struct Ip *ip = (struct Ip *)packet_buffer;
    ip->versionIHL = 0x45;
    ip->dsf = 0;

    int routes = 0;
    for (int r = 0; r < routingTableSize; r++) {
      ip->payload.udp.payload.rip.routes[routes].family = bswap16(2);
      ip->payload.udp.payload.rip.routes[routes].routeTag = 0;
      ip->payload.udp.payload.rip.routes[routes].ip =
          bswap32(routingTable[r].ip);
      ip->payload.udp.payload.rip.routes[routes].netmask =
          bswap32(routingTable[r].netmask);
      ip->payload.udp.payload.rip.routes[routes].nexthop = bswap32(0);
      if (routingTable[r].port != port) {
        // not this port, split horizon
        ip->payload.udp.payload.rip.routes[routes].metric =
            bswap32(routingTable[r].metric);
      } else {
        // from this port, reverse poisoning
        ip->payload.udp.payload.rip.routes[routes].metric = bswap32(16);
      }
      routes++;
    }

    if (routes == 0) {
      routes = 1;
      ip->payload.udp.payload.rip.routes[0].family = 0;
      ip->payload.udp.payload.rip.routes[0].routeTag = 0;
      ip->payload.udp.payload.rip.routes[0].ip = 0;
      ip->payload.udp.payload.rip.routes[0].netmask = 0;
      ip->payload.udp.payload.rip.routes[0].nexthop = 0;
      ip->payload.udp.payload.rip.routes[0].metric = 0;
    }

    u16 totalLength = 20 + 8 + 4 + 20 * routes;

    ip->totalLength = bswap16(totalLength);
    ip->identification = 0;
    ip->flags = 0;
    ip->ttl = 1;
    ip->protocol = 17; // UDP
    memcpy(ip->sourceIP, portIP, 4);
    memcpy(ip->destIP, ripIP, 4);
    ip->payload.udp.srcPort = bswap16(520);
    ip->payload.udp.dstPort = bswap16(520);
    ip->payload.udp.length = bswap16(totalLength - 20);
    ip->payload.udp.checksum = 0;
    ip->payload.udp.payload.rip.command = 2;
    ip->payload.udp.payload.rip.version = 2;
    ip->payload.udp.payload.rip.zero = 0;

    fillIpChecksum(ip);
    HAL_SendIPPacket(port, (u8 *)packet_buffer, totalLength, ripMAC);
  }
}

void handleIP(u8 port, struct Ip *ip, macaddr_t srcMAC) {
  struct Ip *ipResp = (struct Ip *)packet_buffer;
  u32 portIP = 0x0a000001 + (port << 8);
  u32 portIPNet = htonl(portIP);
  u32 destIP;
  memcpy(&destIP, ip->destIP, sizeof(u32));
  destIP = bswap32(destIP);
  if (ip->protocol == 1 && portIP == destIP) {
    // ICMP
    if (ip->payload.icmp.type == 8) {
      // ICMP echo request
      ipResp->versionIHL = 0x45;
      ipResp->dsf = 0;
      u16 totalLength = bswap16(ip->totalLength);
      ipResp->totalLength = ip->totalLength;
      ipResp->identification = 0;
      ipResp->flags = 0;
      ipResp->ttl = 64;
      ipResp->protocol = 1;
      ipResp->headerChecksum = 0;
      memcpy(ipResp->sourceIP, &portIPNet, 4);
      memcpy(ipResp->destIP, ip->sourceIP, 4);
      ipResp->payload.icmp.type = 0;
      ipResp->payload.icmp.code = 0;
      // type: 8 -> 0
      ipResp->payload.icmp.checksum = checksumAdd(ip->payload.icmp.checksum, 8);
      // assuming IHL=5
      memcpy(ipResp->payload.icmp.data, ip->payload.icmp.data,
             totalLength - 20 - 4);

      fillIpChecksum(ipResp);
      HAL_SendIPPacket(port, (u8 *)packet_buffer, totalLength, srcMAC);
    }
  }
}

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
      // if (spi_read_register(84) != 5) {
      // puts("Warning: SPI might not working properly");
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
        // HAL_SendIPPacket(if_index, (uint8_t *)packet, res, src_mac);
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

      // setup routing table
      routingTableSize = 0;
      for (int i = 0; i < 4; i++) {
        routingTable[i].ip = 0x0a000000 + (i << 8);
        routingTable[i].netmask = 0xffffff00;
        routingTable[i].metric = 1;
        routingTable[i].nexthop = 0; // direct route
        routingTable[i].port = i;
        routingTable[i].updateTime = 0;
        routingTable[i].origin = 0; // myself
        routingTableSize++;
      }

      HAL_Init(1, if_addrs);
      uint64_t time = HAL_GetTicks();
      while (1) {
        if (HAL_GetTicks() > time + 1000 * 5) {
          // 5s timer
          xil_printf("Timer\n");
          sendRIPReponse();
          time = HAL_GetTicks();
        }

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

        struct Ip *ip = (struct Ip *)packet;
        handleIP(if_index, ip, src_mac);
      }
    } else {
      puts("Nothing to do\r\n");
    }
  };
}