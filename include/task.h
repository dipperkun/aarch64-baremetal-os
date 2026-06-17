#ifndef __TASK_H__
#define __TASK_H__

#include "types.h"

typedef enum {
    TASK_INVALID = 0,
    TASK_READY = 1,
    TASK_RUNNING = 2,
    TASK_BLOCKED = 3,
    TASK_DONE = 4
} task_state_t;

typedef void (*task_entry_t)(void *arg);

typedef struct {
    uint32_t id;
    task_entry_t entry;
    void *arg;
    task_state_t state;
    uint32_t run_count;
    uint32_t preempt_count;
    uint64_t ticks;
} task_t;

task_t *task_create(task_entry_t entry, void *arg, task_state_t state);
void task_demo(void *arg);
void task_set_state(task_t *task, task_state_t state);
task_state_t task_get_state(task_t *task);

#endif
