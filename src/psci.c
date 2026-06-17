#include "psci.h"
#include "types.h"
#include "uart.h"

#define PSCI_VERSION                0x84000000
#define PSCI_CPU_SUSPEND             0x84000001
#define PSCI_CPU_OFF                 0x84000002
#define PSCI_CPU_ON                  0x84000003
#define PSCI_AFFINITY_INFO           0x84000004
#define PSCI_MIGRATE                 0x84000005
#define PSCI_MIGRATE_INFO_TYPE       0x84000006
#define PSCI_MIGRATE_INFO_UP_CPU     0x84000007
#define PSCI_SYSTEM_OFF              0x84008008
#define PSCI_SYSTEM_RESET            0x84009009

/* PSCI return codes */
#define PSCI_RET_SUCCESS             0
#define PSCI_RET_NOT_SUPPORTED      -1
#define PSCI_RET_INVALID_PARAMS     -2
#define PSCI_RET_DENIED             -3
#define PSCI_RET_ALREADY_ON         -4
#define PSCI_RET_ON_PENDING         -5
#define PSCI_RET_INTERNAL_FAILURE   -6
#define PSCI_RET_NOT_PRESENT        -7
#define PSCI_RET_DISABLED           -8

static int psci_call(uint32_t function, uint64_t arg0, uint64_t arg1, uint64_t arg2)
{
    int ret;
    
    asm volatile(
        "mov x0, %1\n"
        "mov x1, %2\n"
        "mov x2, %3\n"
        "mov x3, %4\n"
        "hvc #0\n"
        "mov %0, x0\n"
        : "=r" (ret)
        : "r" (function), "r" (arg0), "r" (arg1), "r" (arg2)
        : "x0", "x1", "x2", "x3"
    );
    
    return ret;
}

void psci_init(void)
{
    int version = psci_call(PSCI_VERSION, 0, 0, 0);
    
    uart_puts("[PSCI] Version: 0x");
    uart_puthex(version, 8);
    uart_puts("\n");
}

int psci_cpu_on(uint64_t cpuid, uint64_t entry_point)
{
    uart_puts("[PSCI] CPU ON: CPU ");
    uart_puthex(cpuid, 8);
    uart_puts(" at 0x");
    uart_puthex(entry_point, 16);
    uart_puts("\n");
    
    return psci_call(PSCI_CPU_ON, cpuid, entry_point, 0);
}

int psci_cpu_off(void)
{
    uart_puts("[PSCI] CPU OFF\n");
    return psci_call(PSCI_CPU_OFF, 0, 0, 0);
}

int psci_affinity_info(uint64_t target_affinity, uint32_t lowest_affinity_level)
{
    return psci_call(PSCI_AFFINITY_INFO, target_affinity, lowest_affinity_level, 0);
}

void psci_system_off(void)
{
    uart_puts("[PSCI] System OFF\n");
    psci_call(PSCI_SYSTEM_OFF, 0, 0, 0);
}

void psci_system_reset(void)
{
    uart_puts("[PSCI] System RESET\n");
    psci_call(PSCI_SYSTEM_RESET, 0, 0, 0);
}
