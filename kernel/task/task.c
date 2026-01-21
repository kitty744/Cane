#include <valen/task.h>
#include <valen/heap.h>
#include <valen/stdio.h>
#include <valen/string.h>

// Global task management
task_t *current_task = NULL;
static task_t *runqueue = NULL;
static pid_t next_pid = 1;

// Assembly context switch function
extern void switch_to(task_context_t *prev, task_context_t *next);

/**
 * @brief Initialize the task scheduler
 */
void scheduler_init(void) {
    current_task = NULL;
    runqueue = NULL;
    next_pid = 1;
}

/**
 * @brief Add task to runqueue
 */
void add_task_to_runqueue(task_t *task) {
    if (!runqueue) {
        runqueue = task;
        task->next = task;
        task->prev = task;
    } else {
        // Insert at head of circular doubly-linked list
        task->next = runqueue;
        task->prev = runqueue->prev;
        runqueue->prev->next = task;
        runqueue->prev = task;
        runqueue = task;
    }
}

/**
 * @brief Remove task from runqueue
 */
void remove_task_from_runqueue(task_t *task) {
    if (!task || !runqueue) return;
    
    if (task->next == task) {
        // Only task in queue
        runqueue = NULL;
    } else {
        task->prev->next = task->next;
        task->next->prev = task->prev;
        if (runqueue == task) {
            runqueue = task->next;
        }
    }
    task->next = NULL;
    task->prev = NULL;
}

/**
 * @brief Create a new task
 */
task_t *task_create(void (*func)(void), const char *name) {
    task_t *task = (task_t*)malloc(sizeof(task_t));
    if (!task) {
        return NULL;
    }
    
    // Initialize task structure
    memset(task, 0, sizeof(task_t));
    
    task->pid = next_pid++;
    task->state = TASK_RUNNING;
    task->prio = 120;
    task->static_prio = 120;
    task->normal_prio = 120;
    task->rt_priority = 0;
    task->flags = TASK_RUNNING_FLAG;
    task->task_func = func;
    task->exit_code = 0;
    task->parent = current_task;
    
    // Copy command name
    if (name) {
        strncpy(task->comm, name, sizeof(task->comm) - 1);
        task->comm[sizeof(task->comm) - 1] = '\0';
    } else {
        strcpy(task->comm, "unknown");
    }
    
    // Allocate kernel stack
    task->stack_size = 3072;
    task->stack = malloc(task->stack_size);
    if (!task->stack) {
        free(task);
        return NULL;
    }
    
    // Set up initial stack for new task
    uint64_t *stack_top = (uint64_t*)((uint8_t*)task->stack + task->stack_size);
    
    // Align stack to 16-byte boundary
    stack_top = (uint64_t*)((uint64_t)stack_top & ~0xF);
    
    // Set up context structure directly - no need for complex stack frame
    task->context.rsp = (uint64_t)stack_top;
    task->context.rip = (uint64_t)func;
    task->context.cs = 0x08;
    task->context.ss = 0x10;
    task->context.eflags = 0x202;
    
    // Zero out all general purpose registers
    task->context.r15 = 0;
    task->context.r14 = 0;
    task->context.r13 = 0;
    task->context.r12 = 0;
    task->context.rbp = 0;
    task->context.rbx = 0;
    task->context.r11 = 0;
    task->context.r10 = 0;
    task->context.r9 = 0;
    task->context.r8 = 0;
    task->context.rax = 0;
    task->context.rcx = 0;
    task->context.rdx = 0;
    task->context.rsi = 0;
    task->context.rdi = 0;
    task->context.orig_rax = 0;
    
    // Add to runqueue
    add_task_to_runqueue(task);
    
    return task;
}

/**
 * @brief Exit current task
 */
void task_exit(long exit_code) {
    if (!current_task) return;
    
    printf("Task '%s' (PID %d) exiting with code %ld\n", 
           current_task->comm, current_task->pid, exit_code);
    
    current_task->state = TASK_ZOMBIE;
    current_task->exit_code = exit_code;
    
    // Remove from runqueue
    remove_task_from_runqueue(current_task);
    
    // Schedule next task
    schedule();
}

/**
 * @brief Core scheduler
 */
void schedule(void) {
    if (!runqueue) return;
    
    task_t *next = NULL;
    task_t *prev = current_task;
    
    // Find next runnable task
    if (!current_task) {
        next = runqueue;
    } else {
        next = current_task->next;
        if (next == current_task) {
            next = current_task; // Only one task
        }
    }
    
    if (next && next != current_task) {
        
        task_t *old_current = current_task;
        current_task = next;
        
        // Perform context switch
        if (old_current) {
            switch_to(&old_current->context, &next->context);
        } else {
            // First task - set up stack and jump directly
            asm volatile (
                "mov %0, %%rsp\n"
                "jmp *%1"
                :
                : "r" (next->context.rsp), "r" (next->context.rip)
                : "memory"
            );
        }
    }
}

/**
 * @brief Timer tick handler for scheduler
 */
void scheduler_tick(void) {
    if (!current_task) return;
    
    // Simple time slice management
    static int counter = 0;
    counter++;
    
    // Schedule every 25 ticks (0.5 seconds at 50Hz)
    if (counter >= 25) {
        counter = 0;
        schedule();
    }
}

/**
 * @brief Get current task
 */
task_t *get_current_task(void) {
    return current_task;
}

/**
 * @brief Get current PID
 */
pid_t get_current_pid(void) {
    return current_task ? current_task->pid : -1;
}

/**
 * @brief Yield CPU to next task
 */
void yield(void) {
    schedule();
}

/**
 * @brief Find task by PID
 */
task_t *find_task_by_pid(pid_t pid) {
    if (!runqueue || pid <= 0) return NULL;
    
    task_t *current = runqueue;
    do {
        if (current->pid == pid) {
            return current;
        }
        current = current->next;
    } while (current != runqueue);
    
    return NULL;
}

/**
 * @brief Kill a task by PID
 */
int kill_task(pid_t pid) {
    if (pid <= 0) return -1;
    
    task_t *target = find_task_by_pid(pid);
    if (!target) return -1;
    
    // Don't allow killing current task
    if (target == current_task) return -2;
    
    // Mark as zombie and remove from runqueue
    target->state = TASK_ZOMBIE;
    remove_task_from_runqueue(target);
    
    // Free the task's resources
    if (target->stack) {
        free(target->stack);
    }
    free(target);
    
    return 0;
}
