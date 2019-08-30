#include "io.h"

// from
// https://github.com/trivialmips/TrivialMIPS_Software/blob/master/cpp/lib/printf.c
int xil_printf(const char *fmt, ...) {
  int i;
  char c;
  void **arg;
  void *ap;
  int w;
  __builtin_va_start(ap, fmt);
  arg = ap;
  for (i = 0; fmt[i]; i++) {
    c = fmt[i];
    if (c == '%') {
      w = 1;
    again:
      switch (fmt[i + 1]) {
      case 's':
        puts(*arg);
        arg++;
        i++;
        break;
      case 'c':
        putc((long)*arg);
        arg++;
        i++;
        break;
      case 'd':
        putdec((long)*arg);
        arg++;
        i++;
        break;
      case 'p':
      case 'x':
        puthex((long)*arg);
        arg++;
        i++;
        break;
      case '%':
        putc('%');
        i++;
        break;
      case '0':
        i++;
      case '1' ... '9':
        for (w = 0; fmt[i + 1] > '0' && fmt[i + 1] <= '9'; i++)
          w = w * 10 + (fmt[i + 1] - '0');
        goto again;
        break;

      default:
        putc('%');
        break;
      }
    } else {
      if (c == '\n')
        putc('\r');
      putc(c);
    }
  }
  return 0;
}