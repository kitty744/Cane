# Task Management System

## Overview

The Valen task management system provides preemptive multitasking capabilities with a round-robin scheduler. It supports multiple concurrent tasks with proper context switching and timer-driven scheduling.

## Components

### Task Control Block (TCB)

Each task is represented by a `task_t` structure containing:

- **Identification**: PID, command name, state
- **Scheduling**: Priority, scheduling flags, runqueue links
- **Context**: CPU register state for context switching
- **Memory**: Stack allocation and management
- **Process**: Parent task, exit code, task function

```c
typedef struct task {
    pid_t pid;
    char comm[16];
    volatile long state;
    task_context_t context;
    void *stack;
    unsigned long stack_size;
    struct task *next, *prev;
    void (*task_func)(void);
    // ... additional fields
} task_t;
```

### Task States

- **TASK_RUNNING**: Currently executing
- **TASK_INTERRUPTIBLE**: Sleeping/waiting
- **TASK_UNINTERRUPTIBLE**: Uninterruptible sleep
- **TASK_ZOMBIE**: Terminated but not reaped
- **TASK_STOPPED**: Stopped (debugging)
- **TASK_TRACED**: Being traced

### Scheduler

The scheduler implements a simple round-robin algorithm:

1. **Runqueue Management**: Circular doubly-linked list of runnable tasks
2. **Context Switching**: Assembly-level register preservation
3. **Timer Integration**: Preemptive scheduling via PIT interrupts
4. **Task Creation**: Dynamic task allocation and initialization

## API

### Task Management

```c
// Create a new task
task_t *task_create(void (*func)(void), const char *name);

// Exit current task
void task_exit(long exit_code);

// Get current task
task_t *get_current_task(void);

// Get current PID
pid_t get_current_pid(void);

// Yield CPU voluntarily
void yield(void);
```

### Scheduler Control

```c
// Initialize scheduler
void scheduler_init(void);

// Schedule next task
void schedule(void);

// Timer tick handler
void scheduler_tick(void);
```

## Implementation Details

### Context Switching

The context switch is implemented in assembly (`arch/x86_64/context.s`):

1. **Register Preservation**: Saves callee-saved registers (RBP, RBX, R12-R15)
2. **Stack Management**: Updates RSP for each task
3. **Control Transfer**: Jumps to new task's RIP

### Timer Integration

- **PIT Frequency**: 10Hz (100ms intervals)
- **Scheduling**: Every 10 ticks (1 second)
- **Interrupt Handling**: Proper EOI signaling to PIC

### Memory Management

- **Stack Size**: 3072 bytes per task
- **Heap**: Static 4KB heap for task allocation
- **Allocation**: malloc/free for task structures

## Usage Example

```c
void my_task(void) {
    // Task implementation
    while (1) {
        // Do work
        yield(); // Optional: yield CPU
    }
}

void kernel_main(void) {
    // Initialize scheduler
    scheduler_init();

    // Create task
    task_t *task = task_create(my_task, "my_task");

    // Start scheduling
    schedule();
}
```

## Notes

- Tasks run in kernel mode with full privileges
- No user-space separation currently implemented
- Timer frequency can be adjusted via `pit_init()`
- Stack size optimized for 4KB page boundaries
