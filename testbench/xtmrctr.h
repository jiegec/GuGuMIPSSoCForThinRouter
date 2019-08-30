
typedef struct XTmrCtr {
} XTmrCtr;

void XTmrCtr_Initialize(XTmrCtr *tmrCtr, int id) {}
#define XPAR_AXI_TIMER_0_DEVICE_ID 0
void XTmrCtr_Start(XTmrCtr *tmrCtr, int id) {}

uint32_t XTmrCtr_GetValue(XTmrCtr *tmrCtr, int flags) {}
#define XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ 100000000