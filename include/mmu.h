#ifndef __MMU_H__
#define __MMU_H__

#include "types.h"

void mmu_init(void);
void mmu_map_page(uint64_t vaddr, uint64_t paddr, uint32_t flags);

#endif
