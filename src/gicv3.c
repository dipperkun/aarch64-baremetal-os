#include "gicv3.h"
#include "types.h"
#include "uart.h"

/* GICv3 Distributor (GICD) */
#define GICD_BASE 0x08000000

struct gicd_regs {
    volatile uint32_t ctlr;           /* 0x0000 Distributor control */
    volatile uint32_t typer;          /* 0x0004 Controller type */
    volatile uint32_t iidr;           /* 0x0008 Implementer ID */
    volatile uint8_t  _res1[0x74];
    volatile uint32_t statusr;        /* 0x0080 Status register */
    volatile uint8_t  _res2[0x7c];
    volatile uint32_t setspi_nsr;     /* 0x0100 Set SPI */
    volatile uint8_t  _res3[0x3c];
    volatile uint32_t clrspi_nsr;     /* 0x0140 Clear SPI */
    volatile uint8_t  _res4[0xbc];
    volatile uint32_t igroupr[32];    /* 0x0200 Interrupt group */
    volatile uint32_t isenabler[32];  /* 0x0280 Set enable */
    volatile uint32_t icenabler[32];  /* 0x0300 Clear enable */
    volatile uint32_t ispendr[32];    /* 0x0380 Set pending */
    volatile uint32_t icpendr[32];    /* 0x0400 Clear pending */
    volatile uint32_t isactiver[32];  /* 0x0480 Set active */
    volatile uint32_t icactiver[32];  /* 0x0500 Clear active */
    volatile uint32_t ipriorityr[256];/* 0x0580 Priority */
};

static volatile struct gicd_regs *gicd = (struct gicd_regs *)GICD_BASE;

void gicv3_init(void)
{
    uint32_t typer = gicd->typer;
    uint32_t num_irqs = 32 * ((typer & 0x1f) + 1);
    
    uart_puts("[GICv3] Number of interrupts: ");
    uart_puthex(num_irqs, 8);
    uart_puts("\n");
    
    /* Enable distributor */
    gicd->ctlr = 0x37;  /* Enable both Group 0 and Group 1 */
    
    /* Disable all SPIs (Shared Peripheral Interrupts) */
    for (int i = 1; i < 32; i++) {
        gicd->icenabler[i] = 0xffffffff;
    }
    
    /* Clear all pending bits */
    for (int i = 1; i < 32; i++) {
        gicd->icpendr[i] = 0xffffffff;
    }
    
    /* Set priority for all SPIs to 0x80 */
    for (int i = 32; i < num_irqs; i++) {
        gicd->ipriorityr[i] = 0x80;
    }
    
    /* Configure as Group 1 (Non-secure) */
    for (int i = 1; i < 32; i++) {
        gicd->igroupr[i] = 0xffffffff;
    }
    
    uart_puts("[GICv3] Distributor initialized\n");
}

void gicv3_enable_irq(uint32_t irq)
{
    uint32_t regnum = irq / 32;
    uint32_t irq_bit = 1 << (irq % 32);
    
    gicd->isenabler[regnum] = irq_bit;
}

void gicv3_disable_irq(uint32_t irq)
{
    uint32_t regnum = irq / 32;
    uint32_t irq_bit = 1 << (irq % 32);
    
    gicd->icenabler[regnum] = irq_bit;
}

static void gicv3_cpu_init(void)
{
    /* Set up CPU interface (ICC) registers */
    
    /* Enable Group 1 interrupts on this CPU */
    uint64_t icc_grpen1_el1;
    asm volatile("mrs %0, ICC_GRPEN1_EL1" : "=r"(icc_grpen1_el1));
    icc_grpen1_el1 |= 1;
    asm volatile("msr ICC_GRPEN1_EL1, %0" : : "r"(icc_grpen1_el1));
    
    /* Set binary point (no priority grouping for simplicity) */
    asm volatile("msr ICC_BPR1_EL1, %0" : : "r"(0));
    
    /* Set priority mask to allow all interrupts */
    asm volatile("msr ICC_PMR_EL1, %0" : : "r"(0xff));
}

void gicv3_cpu_setup(void)
{
    gicv3_cpu_init();
}
