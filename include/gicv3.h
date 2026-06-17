#ifndef __GICV3_H__
#define __GICV3_H__

#include "types.h"

void gicv3_init(void);
void gicv3_enable_irq(uint32_t irq);
void gicv3_disable_irq(uint32_t irq);
void gicv3_cpu_setup(void);

#endif
