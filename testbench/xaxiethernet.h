typedef struct XAxiEthernet_Config {
  u32 BaseAddress;
} XAxiEthernet_Config;
typedef struct XAxiEthernet {
} XAxiEthernet;
#define XPAR_AXI_ETHERNET_0_DEVICE_ID 0
XAxiEthernet_Config *XAxiEthernet_LookupConfig(int id) {
  static XAxiEthernet_Config config;
  return &config;
}
void XAxiEthernet_CfgInitialize(XAxiEthernet *dma, XAxiEthernet_Config *cfg,
                                u32 addr) {}

#define XAE_RECEIVER_ENABLE_OPTION 0
#define XAE_TRANSMITTER_ENABLE_OPTION 0
#define XAE_VLAN_OPTION 0
void XAxiEthernet_SetOptions(XAxiEthernet *eth, int flags) {}
void XAxiEthernet_SetMacAddress(XAxiEthernet *eth, macaddr_t mac) {}
void XAxiEthernet_Start(XAxiEthernet *eth) {}