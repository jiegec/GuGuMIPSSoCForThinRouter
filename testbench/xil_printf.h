
extern int puts(const char *s);
void xil_printf(char *fmt, ...) { puts(fmt); }