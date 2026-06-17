ENTRY(_start)

MEMORY {
    KERNEL (rwx) : ORIGIN = 0x40000000, LENGTH = 128M
}

SECTIONS {
    .text ALIGN(0x1000) : {
        *(.text.boot)
        *(.text*)
        *(.rodata*)
        . = ALIGN(0x1000);
    } > KERNEL

    .data ALIGN(0x1000) : {
        *(.data*)
        . = ALIGN(0x1000);
    } > KERNEL

    .bss ALIGN(0x1000) : {
        __bss_start = .;
        *(.bss*)
        . = ALIGN(0x1000);
        __bss_end = .;
    } > KERNEL

    /DISCARD/ : {
        *(.note.GNU-stack)
        *(.gnu_debuglink)
        *(.gnu_debugaltlink)
    }
}
