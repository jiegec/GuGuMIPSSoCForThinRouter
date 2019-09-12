typedef struct XSpi {
} XSpi;

extern volatile uint32_t *SPI_RESET;
extern volatile uint32_t *SPI_CONTROL;
extern volatile uint32_t *SPI_STATUS;
extern volatile uint8_t *SPI_TRANSMIT;
extern volatile uint8_t *SPI_RECEIVE;
extern volatile uint32_t *SPI_SLAVESELECT;

void XSpi_SetSlaveSelect(XSpi *spi, uint32_t ss) { *SPI_SLAVESELECT = ~ss; }
#define XSP_MASTER_OPTION (1 << 2)
#define XSP_MANUAL_SSELECT_OPTION (1 << 7)
void XSpi_SetOptions(XSpi *spi, uint32_t opt) { *SPI_CONTROL = (1 << 8) | opt | *SPI_CONTROL; }
void XSpi_Start(XSpi *spi) { *SPI_CONTROL = *SPI_CONTROL | (1 << 1); }
void XSpi_IntrGlobalDisable(XSpi *spi) {}

void XSpi_Transfer(XSpi *spi, u8 *buffer, u8 *outBuffer, uint8_t len) {
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
}