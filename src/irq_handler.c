#include "types.h"
#include "uart.h"
#include "timer.h"

struct exception_frame {
    uint64_t x0, x1, x2, x3, x4, x5, x6, x7;
    uint64_t x8, x9, x10, x11, x12, x13, x14, x15;
    uint64_t x16, x17, x18, x19, x20, x21, x22, x23;
    uint64_t x24, x25, x26, x27, x28, x29, x30;
    uint64_t elr_el1;
    uint64_t spsr_el1;
} __attribute__((packed));

void handle_exception(void)
{
    uint64_t esr_el1, far_el1;
    
    asm volatile("mrs %0, esr_el1" : "=r"(esr_el1));
    asm volatile("mrs %0, far_el1" : "=r"(far_el1));
    
    uint32_t ec = (esr_el1 >> 26) & 0x3f;
    
    uart_puts("[EXCEPTION] Synchronous exception (EC=0x");
    uart_puthex(ec, 2);
    uart_puts(")\n");
    uart_puts("[EXCEPTION] FAR=0x");
    uart_puthex(far_el1, 16);
    uart_puts("\n");
}

void handle_irq(void)
{
    uint64_t iar;
    asm volatile("mrs %0, ICC_IAR1_EL1" : "=r"(iar));
    
    uint32_t irq_id = iar & 0xffffff;
    
    if (irq_id == 1023) {
        /* Spurious interrupt */
        return;
    }
    
    uart_puts("[IRQ] Interrupt ");
    uart_puthex(irq_id, 4);
    uart_puts("\n");
    
    /* Handle specific interrupts */
    if (irq_id == 27) {  /* Generic timer */
        timer_handle_irq();
    }
    
    /* Signal end of interrupt */
    asm volatile("msr ICC_EOIR1_EL1, %0" : : "r"(iar));
}

void handle_fiq(void)
{
    uart_puts("[FIQ] Fast interrupt\n");
}

void handle_serror(void)
{
    uart_puts("[SERROR] System error\n");
}
