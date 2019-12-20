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
u8 portMAC[6] = {2, 2, 3, 3, 0, 0};
volatile uint32_t *ROUTING_TABLE = (uint32_t *)0xBFB00000;

int strequ(char *a, char *b) {
  while (*a && *b) {
    if (*a++ != *b++) {
      return 0;
    }
  }
  return *a == *b;
}

void printIP(u32 ip) {
  int p1 = ip >> 24, p2 = (ip >> 16) & 0xFF, p3 = (ip >> 8) & 0xFF,
      p4 = ip & 0xFF;
  xil_printf("%d.%d.%d.%d", p1, p2, p3, p4);
}

in_addr_t if_addrs[N_IFACE_ON_BOARD] = {0xc0a80001, 0xc0a80101, 0xc0a80201,
                                        0xc0a80301};

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

void fillIcmpChecksum(struct Icmp *icmp, u32 length) {
  icmp->checksum = 0;
  u16 *data = ((u16 *)icmp);
  u16 checksum = 0;
  for (int i = 0; i < length / 2; i++) {
    checksum = checksumAdd(checksum, data[i]);
  }
  icmp->checksum = ~checksum;
}

void sendRIPResponse() {
  // send RIP response
  for (u8 port = 0; port < 4; port++) {
    u32 portIP = bswap32(if_addrs[port]);
    u8 ripIP[4] = {224, 0, 0, 9};
    for (int part = 0; part < (routingTableSize + 24) / 25; part++) {
      int from = part * 25;
      int to = (part + 1) * 25;
      if (to > routingTableSize) {
        to = routingTableSize;
      }

      struct Ip *ip = (struct Ip *)packet_buffer;
      ip->versionIHL = 0x45;
      ip->dsf = 0;

      int routes = 0;
      for (int r = from; r < to; r++) {
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
      memcpy(ip->sourceIP, &portIP, 4);
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
}

void handleIP(u8 port, struct Ip *ip, macaddr_t srcMAC) {
  struct Ip *ipResp = (struct Ip *)packet_buffer;
  u32 portIP = if_addrs[port];
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
  } else if (ip->protocol == 17) {
    // UDP
    if (ip->payload.udp.srcPort == bswap16(520) &&
        ip->payload.udp.dstPort == bswap16(520)) {
      // RIP
      // xil_printf("Got RIP response from port %d:\n", port);
      u16 totalLength = bswap16(ip->totalLength);
      u32 sourceIP;
      memcpy(&sourceIP, ip->sourceIP, sizeof(u32));
      sourceIP = bswap32(sourceIP);
      int totalRoutes = (totalLength - 20 - 8 - 4) / 20;
      for (int routes = 0; routes < totalRoutes; routes++) {
        u32 ip_net = bswap32(ip->payload.udp.payload.rip.routes[routes].ip);
        u32 netmask =
            bswap32(ip->payload.udp.payload.rip.routes[routes].netmask);
        u32 nexthop =
            bswap32(ip->payload.udp.payload.rip.routes[routes].nexthop);
        if (nexthop == 0) {
          memcpy(&nexthop, ip->sourceIP, 4);
          nexthop = bswap32(nexthop);
        }
        u32 metric = bswap32(ip->payload.udp.payload.rip.routes[routes].metric);

        // xil_printf("\t%d: ", routes);
        // printIP(ip_net);
        // xil_printf(" netmask ");
        // printIP(netmask);
        // xil_printf(" nexthop ");
        // printIP(nexthop);
        // xil_printf(" metric %d\n", metric);

        metric += 1;
        if (metric > 16) {
          metric = 16;
        }

        int flag = 0;
        for (int i = 0; i < routingTableSize; i++) {
          if (routingTable[i].ip == ip_net &&
              routingTable[i].netmask == netmask) {
            if (routingTable[i].origin == sourceIP) {
              routingTable[i].updateTime = HAL_GetTicks() / 1000;
            }
            if (metric < routingTable[i].metric ||
                memcmp(ip->sourceIP, &routingTable[i].origin, 4) == 0) {
              // update this entry
              if (metric >= 16) {
                // remove this entry
                memmove(&routingTable[i], &routingTable[i + 1],
                        sizeof(struct Route) * (routingTableSize - i - 1));
                routingTableSize--;
                i--;
              } else {
                routingTable[i].metric = metric;
                routingTable[i].nexthop = nexthop;
                routingTable[i].port = port;
                routingTable[i].updateTime = HAL_GetTicks() / 1000;
              }
            }
            flag = 1;
            break;
          }
        }

        if (!flag && metric < 16) {
          // add this entry
          routingTable[routingTableSize].ip = ip_net;
          routingTable[routingTableSize].netmask = netmask;
          routingTable[routingTableSize].nexthop = nexthop;
          routingTable[routingTableSize].port = port;
          routingTable[routingTableSize].metric = metric;
          routingTable[routingTableSize].updateTime = HAL_GetTicks() / 1000;
          routingTable[routingTableSize].origin = sourceIP;
          routingTableSize++;
        }
      }
    }
  } else if (ip->ttl == 1) {
    // send ICMP Time Exceeded
    ipResp->versionIHL = 0x45;
    ipResp->dsf = 0;
    u16 totalLength = 20 + 4 + 4 + 20 + 8;
    ipResp->totalLength = bswap16(totalLength);
    ipResp->identification = 0;
    ipResp->flags = 0;
    ipResp->ttl = 64;
    ipResp->protocol = 1;
    ipResp->headerChecksum = 0;
    memcpy(ipResp->sourceIP, &portIPNet, 4);
    memcpy(ipResp->destIP, ip->sourceIP, 4);
    ipResp->payload.icmp.type = 11; // Time exceeded
    ipResp->payload.icmp.code = 0;
    ipResp->payload.icmp.checksum = 0;
    // unused
    memset(ipResp->payload.icmp.data, 0, 4);
    // assuming IHL=5
    memcpy(ipResp->payload.icmp.data + 4, &ip->versionIHL, 20 + 8);

    fillIcmpChecksum(&ipResp->payload.icmp, 4 + 4 + 20 + 8);
    fillIpChecksum(ipResp);
    HAL_SendIPPacket(port, buffer, totalLength, srcMAC);
  }
}

int routingTableCmp(const void *a, const void *b) {
  struct Route *aa = (struct Route *)a;
  struct Route *bb = (struct Route *)b;
  // unreachable last
  if (aa->metric < 16 && bb->metric >= 16) {
    return -1;
  } else if (aa->metric >= 16 && bb->metric < 16) {
    return 1;
  }
  // largest netmask first
  if (aa->netmask > bb->netmask)
    return -1;
  else if (aa->netmask < bb->netmask)
    return 1;

  // smallest ip first
  if (aa->ip < bb->ip)
    return -1;
  else if (aa->ip > bb->ip)
    return 1;
  return 0;
}

void applyCurrentRoutingTable() {
  qsort(routingTable, routingTableSize, sizeof(struct Route), routingTableCmp);
  // add all-zero route as the end
  for (int i = 0; i < 4; i++) {
    *(ROUTING_TABLE + routingTableSize * 4 + i) = 0;
  }
  for (int i = routingTableSize - 1; i >= 0; i--) {
    if (routingTable[i].metric >= 16) {
      *(ROUTING_TABLE + i * 4 + 0) = 0;
      *(ROUTING_TABLE + i * 4 + 1) = 0;
      *(ROUTING_TABLE + i * 4 + 2) = 0;
      *(ROUTING_TABLE + i * 4 + 3) = 0;
    } else {
      *(ROUTING_TABLE + i * 4 + 0) = routingTable[i].nexthop;
      *(ROUTING_TABLE + i * 4 + 1) = routingTable[i].netmask;
      *(ROUTING_TABLE + i * 4 + 2) = routingTable[i].ip;
      *(ROUTING_TABLE + i * 4 + 3) = routingTable[i].port;
    }
  }
}

u32 all_routes[1024][4];
void printCurrentRoutingTable() {
  u32 offset = 0;
  int j = 0;
  for (int flag = 1; flag && j < 1024; j++) {
    u32 route[4];
    flag = 0;
    for (u32 i = 0; i < 4; i++) {
      route[i] = *(ROUTING_TABLE + offset + i);
      if (route[i]) {
        flag = 1;
      }
    }
    offset += 4;
    memcpy(all_routes[j], route, sizeof(route));
  }
  j--;
  xil_printf("Hardware table:\n");
  for (int i = 0; i < j; i++) {
    xil_printf("\t%d: ", i);
    printIP(all_routes[i][2]);
    xil_printf(" netmask ");
    printIP(all_routes[i][1]);
    xil_printf(" via ");
    printIP(all_routes[i][0]);
    xil_printf(" dev port%d\n", all_routes[i][3]);
  }
  xil_printf("Software table:\n");
  for (int i = 0; i < routingTableSize; i++) {
    if (routingTable[i].nexthop != 0) {
      // indirect
      xil_printf("\t%d: ", i);
      printIP(routingTable[i].ip);
      xil_printf(" netmask ");
      printIP(routingTable[i].netmask);
      xil_printf(" via ");
      printIP(routingTable[i].nexthop);
      xil_printf(" dev port%d metric %d timer %d learned from ",
                 routingTable[i].port, routingTable[i].metric,
                 ((u32)HAL_GetTicks()) / 1000 - routingTable[i].updateTime);
      printIP(routingTable[i].origin);
      xil_printf("\n");
    } else {
      xil_printf("\t%d: ", i);
      printIP(routingTable[i].ip);
      xil_printf(" netmask ");
      printIP(routingTable[i].netmask);
      xil_printf(" dev port%d\n", routingTable[i].port);
    }
  }
  applyCurrentRoutingTable();
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
        routingTable[i].ip = if_addrs[i] & 0xffffff00;
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
          sendRIPResponse();
          printCurrentRoutingTable();
          time = HAL_GetTicks();
        }

        macaddr_t src_mac;
        macaddr_t dst_mac;
        int if_index;
        int res = HAL_ReceiveIPPacket((1 << N_IFACE_ON_BOARD) - 1,
                                      (uint8_t *)packet, sizeof(packet),
                                      src_mac, dst_mac, 1000, &if_index);
        if (0) {
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
        }

        struct Ip *ip = (struct Ip *)packet;
        handleIP(if_index, ip, src_mac);
      }
    } else {
      puts("Nothing to do\r\n");
    }
  };
}