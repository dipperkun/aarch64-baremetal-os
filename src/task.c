#include "task.h"
#include "uart.h"
#include "types.h"

#define MAX_TASKS 256
static task_t task_pool[MAX_TASKS];
static int task_count = 0;

task_t *task_create(task_entry_t entry, void *arg, task_state_t state)
{
    if (task_count >= MAX_TASKS) {
        uart_puts("[TASK] ERROR: Task pool exhausted\n");
        return NULL;
    }
    
    task_t *task = &task_pool[task_count++];
    task->id = task_count - 1;
    task->entry = entry;
    task->arg = arg;
    task->state = state;
    task->run_count = 0;
    task->preempt_count = 0;
    task->ticks = 0;
    
    return task;
}

void task_demo(void *arg)
{
    uint64_t id = (uint64_t)arg;
    
    uart_puts("[TASK ");
    uart_puthex(id, 2);
    uart_puts("] Starting\n");
    
    for (int i = 0; i < 5; i++) {
        uart_puts("[TASK ");
        uart_puthex(id, 2);
        uart_puts("] Running iteration ");
        uart_puthex(i, 2);
        uart_puts("\n");
        
        /* Simulate work */
        volatile uint64_t count = 0;
        for (uint64_t j = 0; j < 1000000; j++) {
            count++;
        }
    }
    
    uart_puts("[TASK ");
    uart_puthex(id, 2);
    uart_puts("] Completed\n");
}

void task_set_state(task_t *task, task_state_t state)
{
    if (task) {
        task->state = state;
    }
}

task_state_t task_get_state(task_t *task)
{
    if (task) {
        return task->state;
    }
    return TASK_INVALID;
}
