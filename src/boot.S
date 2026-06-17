#include "asm/macro.h"

.section .text.boot
.globl _start
.align 2

_start:
    /* Get CPU ID (MPIDR_EL1) */
    mrs     x0, mpidr_el1
    and     x0, x0, #0xff
    
    /* If CPU 0, initialize */
    cbnz    x0, _secondary_cpu

    /* Primary CPU: initialize stack and kernel */
    adrp    x1, __bss_end
    add     x1, x1, #0x10000          /* Stack at end of BSS + 64KB */
    mov     sp, x1
    
    /* Clear BSS section */
    adrp    x1, __bss_start
    adrp    x2, __bss_end
    
.clear_bss:
    cmp     x1, x2
    beq     .bss_done
    str     xzr, [x1], #8
    b       .clear_bss

.bss_done:
    /* Set EL1 exception vectors */
    adrp    x0, exc_vector_table
    add     x0, x0, :lo12:exc_vector_table
    msr     vbar_el1, x0
    
    /* Call main kernel */
    bl      kernel_main
    
    /* Halt */
    b       .

_secondary_cpu:
    /* Secondary CPUs wait here */
    mov     x1, #0x40010000            /* CPU release address for PSCI */
    ldr     x2, [x1, x0, lsl #3]
    cbz     x2, _secondary_cpu
    
    /* Set up stack for secondary CPU */
    adrp    x3, __bss_end
    mov     x1, #0x4000
    madd    x1, x0, x1, x3
    mov     sp, x1
    
    /* Enter secondary CPU handler */
    br      x2
    b       .

/* Exception Vector Table */
.section .text
.align 11
.globl exc_vector_table
exc_vector_table:
    /* Current EL with SP0 */
    .align 7
    b       exc_sync
    .align 7
    b       exc_irq
    .align 7
    b       exc_fiq
    .align 7
    b       exc_serror
    
    /* Current EL with SPx */
    .align 7
    b       exc_sync
    .align 7
    b       exc_irq
    .align 7
    b       exc_fiq
    .align 7
    b       exc_serror
    
    /* Lower EL 64-bit */
    .align 7
    b       exc_sync
    .align 7
    b       exc_irq
    .align 7
    b       exc_fiq
    .align 7
    b       exc_serror
    
    /* Lower EL 32-bit */
    .align 7
    b       exc_sync
    .align 7
    b       exc_irq
    .align 7
    b       exc_fiq
    .align 7
    b       exc_serror

exc_sync:
    b       handle_exception

exc_irq:
    b       handle_irq

exc_fiq:
    b       handle_fiq

exc_serror:
    b       handle_serror
