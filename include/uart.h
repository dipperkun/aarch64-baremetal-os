#ifndef __UART_H__
#define __UART_H__

#include "types.h"

void uart_init(void);
void uart_putc(char c);
char uart_getc(void);
int uart_readable(void);
void uart_puts(const char *str);
void uart_puthex(uint64_t value, int width);

#endif
