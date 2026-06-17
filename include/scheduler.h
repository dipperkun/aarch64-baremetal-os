#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "types.h"
#include "task.h"

void scheduler_init(void);
void scheduler_add_task(task_t *task);
void scheduler_tick(void);
void scheduler_start(void) __attribute__((noreturn));
void scheduler_cpu_ready(void);
task_t *scheduler_get_current(void);

#endif
