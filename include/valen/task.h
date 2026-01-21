#ifndef VALEN_TASK_H
#define VALEN_TASK_H

#include <stdint.h>
#include <stdbool.h>

// Process ID type
typedef int pid_t;

// Task states
typedef enum {
    TASK_RUNNING = 0,
    TASK_INTERRUPTIBLE,
    TASK_UNINTERRUPTIBLE,
    TASK_ZOMBIE,
    TASK_STOPPED,
    TASK_TRACED
} task_state_t;

// Task flags
#define TASK_RUNNING_FLAG    0x00000001
#define TASK_INTERRUPTIBLE_FLAG 0x00000002
#define TASK_UNINTERRUPTIBLE_FLAG 0x00000004
#define TASK_ZOMBIE_FLAG    0x00000008

// Process context structure
typedef struct task_context {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rax;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t orig_rax;
    uint64_t rip;
    uint64_t cs;
    uint64_t eflags;
    uint64_t rsp;
    uint64_t ss;
} task_context_t;

// Task Control Block
typedef struct task {
    // Task identification
    volatile long state;
    pid_t pid;
    char comm[16];  // Command name
    
    // Scheduling information
    int prio;
    int static_prio;
    int normal_prio;
    unsigned int rt_priority;
    
    // Task context
    task_context_t context;
    
    // Stack information
    void *stack;
    unsigned long stack_size;
    
    // Linked list for runqueue
    struct task *next;
    struct task *prev;
    
    // Task function
    void (*task_func)(void);
    
    // Exit information
    long exit_code;
    struct task *parent;
    
    // Flags
    unsigned int flags;
} task_t;

// Global current task
extern task_t *current_task;

// Core task management functions
task_t *task_create(void (*func)(void), const char *name);
void task_exit(long exit_code);
void schedule(void);
void context_switch(task_t *prev, task_t *next);

// Assembly functions
extern void switch_to(task_context_t *prev, task_context_t *next);

// Scheduler functions
void scheduler_init(void);
void scheduler_tick(void);
void add_task_to_runqueue(task_t *task);
void remove_task_from_runqueue(task_t *task);

// Utility functions
task_t *get_current_task(void);
pid_t get_current_pid(void);
void yield(void);
task_t *find_task_by_pid(pid_t pid);
int kill_task(pid_t pid);

#endif // VALEN_TASK_H
