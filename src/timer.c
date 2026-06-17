#include "timer.h"
#include "uart.h"
#include "types.h"

#define TIMER_INTERVAL 10000000  /* 10ms in nanoseconds */

void timer_init(void)
{
    /* Get the counter frequency */
    uint32_t cntfrq;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq));
    
    uart_puts("[TIMER] Counter frequency: ");
    uart_puthex(cntfrq, 8);
    uart_puts(" Hz\n");
    
    /* Calculate compare value (next timer interrupt) */
    uint64_t cntpct;
    asm volatile("mrs %0, cntpct_el0" : "=r"(cntpct));
    
    uint64_t interval = (cntfrq / 1000) * 10;  /* 10ms */
    uint64_t cmp_value = cntpct + interval;
    
    /* Set physical timer comparison value */
    asm volatile("msr cntp_cval_el0, %0" : : "r"(cmp_value));
    
    /* Enable physical timer and unmask IRQ */
    uint32_t ctl = 1;  /* Enable bit */
    asm volatile("msr cntp_ctl_el0, %0" : : "r"(ctl));
    
    uart_puts("[TIMER] Physical timer enabled\n");
}

void timer_handle_irq(void)
{
    uart_puts("[TIMER] Tick\n");
    
    /* Get current counter value */
    uint64_t cntpct;
    asm volatile("mrs %0, cntpct_el0" : "=r"(cntpct));
    
    /* Schedule next interrupt */
    uint32_t cntfrq;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq));
    
    uint64_t interval = (cntfrq / 1000) * 10;  /* 10ms */
    uint64_t next_cmp = cntpct + interval;
    
    asm volatile("msr cntp_cval_el0, %0" : : "r"(next_cmp));
}

uint64_t timer_get_ticks(void)
{
    uint64_t ticks;
    asm volatile("mrs %0, cntpct_el0" : "=r"(ticks));
    return ticks;
}
