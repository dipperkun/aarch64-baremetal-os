#ifndef __TIMER_H__
#define __TIMER_H__

#include "types.h"

void timer_init(void);
void timer_handle_irq(void);
uint64_t timer_get_ticks(void);

#endif
