#ifndef __PSCI_H__
#define __PSCI_H__

#include "types.h"

void psci_init(void);
int psci_cpu_on(uint64_t cpuid, uint64_t entry_point);
int psci_cpu_off(void);
int psci_affinity_info(uint64_t target_affinity, uint32_t lowest_affinity_level);
void psci_system_off(void);
void psci_system_reset(void);

#endif
