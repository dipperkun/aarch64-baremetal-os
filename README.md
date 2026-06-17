# ARMv8 Baremetal OS

A complete baremetal operating system for ARMv8 (aarch64) running on QEMU virt board.

## Features

1. **SMP (Symmetric Multi-Processing)** - Multi-core support for up to 4 CPUs
2. **MMU (Memory Management Unit)** - Virtual memory with 48-bit address space and paging
3. **UART** - PL011 serial console driver (115200 baud)
4. **GICv3** - ARM Generic Interrupt Controller v3 for interrupt management
5. **Preemptible Scheduler** - Round-robin task scheduler with time slicing
6. **PSCI** - Power State Coordination Interface for CPU power management
7. **Multitasking** - Support for multiple concurrent tasks with context switching

## Architecture

- **Processor**: ARM Cortex-A53 (ARMv8-A)
- **Board**: QEMU virt board
- **Memory**: 512MB
- **Boot**: EL3 -> EL1 transition

## Building

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu qemu-system-arm

# macOS (with Homebrew)
brew install aarch64-elf-binutils qemu
```

### Build

```bash
make clean
make
```

This will generate:
- `os.elf` - Kernel executable
- `os.bin` - Binary kernel image
- `os.dis` - Disassembly listing

## Running

```bash
make run
```

Expected output:
```
================================
  ARMv8 Baremetal OS Kernel
  QEMU virt board (cortex-a53)
================================

[KERNEL] Initializing kernel subsystems...
[KERNEL] Setting up MMU...
[KERNEL] MMU initialized
[KERNEL] Initializing GICv3...
[GICv3] Number of interrupts: 192
[GICv3] Distributor initialized
[KERNEL] Setting up timer...
[TIMER] Counter frequency: 62500000 Hz
[TIMER] Physical timer enabled
[KERNEL] Initializing scheduler...
[KERNEL] Scheduler initialized
[KERNEL] Initializing PSCI...
[PSCI] Version: 0x10000
[KERNEL] Creating demo tasks...
[SCHEDULER] Task added: ...
[KERNEL] Demo tasks created
[KERNEL] Enabling interrupts...
[KERNEL] Interrupts enabled

[KERNEL] Kernel ready! Starting scheduler...

[SCHEDULER] Starting task scheduling
[TASK 00] Starting
[TASK 00] Running iteration 00
...
```

## Debugging

Start QEMU with GDB stub:

```bash
make debug
```

In another terminal:

```bash
aarch64-linux-gnu-gdb os.elf
(gdb) target remote :1234
(gdb) break kernel_main
(gdb) continue
```

## Project Structure

```
.
├── Makefile              # Build system
├── linker.ld            # Linker script
├── README.md            # This file
├── include/             # Header files
│   ├── kernel.h        # Kernel definitions
│   ├── uart.h          # Serial driver
│   ├── mmu.h           # Virtual memory
│   ├── gicv3.h         # Interrupt controller
│   ├── scheduler.h     # Task scheduler
│   ├── psci.h          # Power management
│   ├── task.h          # Task management
│   ├── timer.h         # Timer driver
│   ├── types.h         # Type definitions
│   └── asm/macro.h     # Assembly macros
└── src/                 # Source files
    ├── boot.S          # Boot code
    ├── main.c          # Kernel entry point
    ├── uart.c          # UART implementation
    ├── mmu.c           # MMU setup
    ├── gicv3.c         # GICv3 implementation
    ├── scheduler.c     # Scheduler implementation
    ├── psci.c          # PSCI implementation
    ├── task.c          # Task management
    ├── irq_handler.c   # Exception/IRQ handlers
    └── timer.c         # Timer implementation
```

## Key Components

### Boot (boot.S)
- CPU startup code
- Exception vector table
- EL3 to EL1 transition
- BSS initialization
- Secondary CPU wake-up

### MMU (mmu.c)
- 4-level page table setup
- Virtual address translation
- Cache configuration
- 48-bit address space support

### GICv3 (gicv3.c)
- Interrupt distributor setup
- IRQ enable/disable
- CPU interface configuration
- Priority management

### Scheduler (scheduler.c)
- Round-robin task scheduling
- Preemption via timer tick
- Task state management
- Multi-CPU support

### PSCI (psci.c)
- CPU power on/off
- System reset/shutdown
- Affinity queries

### Timer (timer.c)
- Generic ARM timer setup
- Periodic tick generation
- Interrupt handling

## Memory Map

```
0xffffffff_ffffffff +----+
                    | VM |
0xffff0000_00000000 +----+ <- Kernel virtual base
                    |
                    | 128MB kernel space
                    |
0x40000000 +----+ <- Kernel physical base (0x40000000)
           |    | 512MB system RAM
0x00000000 +----+
```

## Interrupt Handling

- **IRQ 27**: Generic timer interrupt (triggers task preemption)
- **GICv3 groups**: Group 1 (Non-secure) for application interrupts
- **Priority**: 0x80 for all SPIs (Shared Peripheral Interrupts)

## Future Enhancements

- [ ] Dynamic memory allocation (heap)
- [ ] Virtual filesystem
- [ ] User mode support (EL0)
- [ ] Process isolation
- [ ] Synchronization primitives (semaphores, mutexes)
- [ ] IPC mechanisms
- [ ] Device tree parsing
- [ ] SMP CPU bring-up

## References

- ARM Architecture Reference Manual ARMv8
- QEMU virt board documentation
- GICv3 specification
- ARM PSCI specification

## License

MIT License
