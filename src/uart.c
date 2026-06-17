#include "uart.h"
#include "types.h"

#define UART_BASE 0x09000000  /* PL011 UART on virt board */

struct pl011_regs {
    volatile uint32_t dr;      /* 0x00 Data register */
    volatile uint32_t rsrecr;  /* 0x04 Receive status/error */
    volatile uint8_t  _unused[0x10];
    volatile uint32_t fr;      /* 0x18 Flag register */
    volatile uint8_t  _unused2[0x4];
    volatile uint32_t ibrd;    /* 0x24 Integer baud rate */
    volatile uint32_t fbrd;    /* 0x28 Fractional baud rate */
    volatile uint32_t lcr_h;   /* 0x2C Line control register */
    volatile uint32_t cr;      /* 0x30 Control register */
    volatile uint32_t ifls;    /* 0x34 Interrupt FIFO level */
    volatile uint32_t imsc;    /* 0x38 Interrupt mask */
    volatile uint32_t ris;     /* 0x3C Raw interrupt status */
    volatile uint32_t mis;     /* 0x40 Masked interrupt status */
    volatile uint32_t icr;     /* 0x44 Interrupt clear */
};

static volatile struct pl011_regs *uart = (struct pl011_regs *)UART_BASE;

void uart_init(void)
{
    /* Disable UART */
    uart->cr = 0;
    
    /* Set baud rate: 115200 at 24MHz clock
     * IBRD = 24000000 / (16 * 115200) = 13
     * FBRD = (0.0434 * 64) + 0.5 = 3
     */
    uart->ibrd = 13;
    uart->fbrd = 3;
    
    /* 8N1 (8 data bits, no parity, 1 stop bit), enable FIFOs */
    uart->lcr_h = 0x70;  /* 0b01110000 */
    
    /* Enable UART, RX, TX */
    uart->cr = 0x301;  /* 0b1100000001 */
}

void uart_putc(char c)
{
    /* Wait until transmit FIFO has space */
    while (uart->fr & 0x20)  /* Check TXFF (bit 5) */
        ;
    
    uart->dr = c;
}

char uart_getc(void)
{
    /* Wait until receive FIFO has data */
    while (uart->fr & 0x10)  /* Check RXFE (bit 4) */
        ;
    
    return uart->dr & 0xFF;
}

int uart_readable(void)
{
    return !(uart->fr & 0x10);  /* Return true if RXFE is clear */
}

void uart_puts(const char *str)
{
    while (*str) {
        uart_putc(*str++);
    }
}

void uart_puthex(uint64_t value, int width)
{
    const char *hex = "0123456789abcdef";
    
    for (int i = (width * 4) - 4; i >= 0; i -= 4) {
        uart_putc(hex[(value >> i) & 0xf]);
    }
}
