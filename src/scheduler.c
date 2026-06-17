#include "scheduler.h"
#include "task.h"
#include "uart.h"
#include "types.h"

#define MAX_TASKS 256
#define TASK_TIME_SLICE 10000000  /* 10ms in nanoseconds */

typedef struct {
    task_t *tasks[MAX_TASKS];
    int num_tasks;
    int current_task;
    uint64_t tick_count;
} scheduler_t;

static scheduler_t scheduler = {0};
static task_t *current_task = NULL;

void scheduler_init(void)
{
    scheduler.num_tasks = 0;
    scheduler.current_task = 0;
    scheduler.tick_count = 0;
    current_task = NULL;
}

void scheduler_add_task(task_t *task)
{
    if (scheduler.num_tasks >= MAX_TASKS) {
        uart_puts("[SCHEDULER] ERROR: Too many tasks\n");
        return;
    }
    
    scheduler.tasks[scheduler.num_tasks++] = task;
    uart_puts("[SCHEDULER] Task added: ");
    uart_puthex((uint64_t)task, 16);
    uart_puts("\n");
}

static task_t *get_next_task(void)
{
    if (scheduler.num_tasks == 0) {
        return NULL;
    }
    
    /* Simple round-robin scheduler */
    task_t *task = NULL;
    int attempts = 0;
    
    while (attempts < scheduler.num_tasks) {
        scheduler.current_task = (scheduler.current_task + 1) % scheduler.num_tasks;
        task = scheduler.tasks[scheduler.current_task];
        
        if (task && task->state == TASK_READY) {
            return task;
        }
        
        attempts++;
    }
    
    return NULL;
}

void scheduler_tick(void)
{
    scheduler.tick_count++;
    
    if ((scheduler.tick_count % 1000) == 0) {
        /* Preempt current task */
        if (current_task) {
            current_task->state = TASK_READY;
            current_task->preempt_count++;
        }
    }
}

void scheduler_start(void)
{
    uart_puts("[SCHEDULER] Starting task scheduling\n");
    
    while (1) {
        task_t *task = get_next_task();
        
        if (!task) {
            uart_puts("[SCHEDULER] No runnable tasks, halting\n");
            asm volatile("wfi");
            continue;
        }
        
        current_task = task;
        task->state = TASK_RUNNING;
        task->run_count++;
        
        /* Save current context and switch to task */
        if (task->entry) {
            task->entry(task->arg);
            task->state = TASK_DONE;
        }
    }
}

void scheduler_cpu_ready(void)
{
    /* Mark secondary CPU as ready */
    uart_puts("[SCHEDULER] CPU ready\n");
}

task_t *scheduler_get_current(void)
{
    return current_task;
}
