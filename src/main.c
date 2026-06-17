#include "kernel.h"
#include "uart.h"
#include "mmu.h"
#include "gicv3.h"
#include "scheduler.h"
#include "psci.h"
#include "timer.h"
#include "task.h"

void print_banner(void)
{
    uart_puts("\n");
    uart_puts("================================\n");
    uart_puts("  ARMv8 Baremetal OS Kernel\n");
    uart_puts("  QEMU virt board (cortex-a53)\n");
    uart_puts("================================\n\n");
}

void create_demo_tasks(void)
{
    /* Create 4 demo tasks */
    for (int i = 0; i < 4; i++) {
        task_t *task = task_create(task_demo, (void *)(uintptr_t)i, TASK_READY);
        if (task) {
            scheduler_add_task(task);
        }
    }
}

void kernel_main(void)
{
    /* Initialize UART first for debug output */
    uart_init();
    
    print_banner();
    uart_puts("[KERNEL] Initializing kernel subsystems...\n");
    
    /* Initialize MMU (virtual memory) */
    uart_puts("[KERNEL] Setting up MMU...\n");
    mmu_init();
    uart_puts("[KERNEL] MMU initialized\n");
    
    /* Initialize GICv3 (interrupt controller) */
    uart_puts("[KERNEL] Initializing GICv3...\n");
    gicv3_init();
    uart_puts("[KERNEL] GICv3 initialized\n");
    
    /* Initialize timer */
    uart_puts("[KERNEL] Setting up timer...\n");
    timer_init();
    uart_puts("[KERNEL] Timer initialized\n");
    
    /* Initialize scheduler */
    uart_puts("[KERNEL] Initializing scheduler...\n");
    scheduler_init();
    uart_puts("[KERNEL] Scheduler initialized\n");
    
    /* Initialize PSCI (Power State Coordination) */
    uart_puts("[KERNEL] Initializing PSCI...\n");
    psci_init();
    uart_puts("[KERNEL] PSCI initialized\n");
    
    /* Create demo tasks */
    uart_puts("[KERNEL] Creating demo tasks...\n");
    create_demo_tasks();
    uart_puts("[KERNEL] Demo tasks created\n");
    
    /* Enable interrupts */
    uart_puts("[KERNEL] Enabling interrupts...\n");
    asm volatile("msr daifclr, #2");  /* Enable IRQ (clear bit 1 of DAIF) */
    uart_puts("[KERNEL] Interrupts enabled\n");
    
    uart_puts("\n[KERNEL] Kernel ready! Starting scheduler...\n\n");
    
    /* Start scheduler - never returns */
    scheduler_start();
    
    /* Halt */
    while (1) {
        asm volatile("wfi");  /* Wait for interrupt */
    }
}

void secondary_cpu_init(void)
{
    uart_puts("[CPU] Secondary CPU online\n");
    
    /* Enable interrupts on secondary CPU */
    asm volatile("msr daifclr, #2");
    
    /* Mark CPU as ready in scheduler */
    scheduler_cpu_ready();
    
    /* Halt - scheduler will manage this CPU */
    while (1) {
        asm volatile("wfi");
    }
}
