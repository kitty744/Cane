/**
 * @file shell.c
 * @brief Valen Interactive Shell
 * * Provides a command-line interface for system interaction. This module
 * manages a local input buffer, handles character insertion/deletion,
 * and interfaces with VGA driver to provide a flicker-free experience
 * through hardware cursor masking and coordinate math.
 */

#include <valen/stdio.h>
#include <valen/string.h>
#include <valen/io.h>
#include <valen/pmm.h>
#include <valen/heap.h>
#include <valen/task.h>
#include <valen/spinlock.h>
#include <valen/color.h>

#define MAX_BUFFER 256
#define PROMPT "valen >> "
#define PROMPT_LEN 9

static char input_buffer[MAX_BUFFER];
static int buffer_len = 0;
static int cursor_idx = 0;
static int prompt_start_y = 1;
static spinlock_t shell_lock = SPINLOCK_INIT;

/**
 * @brief Resets shell state and initializes prompt.
 * Clears the internal input buffer, resets the logical cursor index,
 * and establishes the vertical anchor (prompt_start_y) for the current
 * command line.
 */
void shell_init()
{
    spinlock_acquire(&shell_lock);
    
    memset(input_buffer, 0, MAX_BUFFER);
    buffer_len = 0;
    cursor_idx = 0;

    if (get_cursor_y() < 1)
        set_cursor(0, 1);

    prompt_start_y = get_cursor_y();
    puts(PROMPT);
    
    spinlock_release(&shell_lock);
}

/**
 * @brief Redraws the command line while preventing cursor ghosting.
 * This function masks the hardware cursor during the printing process.
 * It calculates the absolute VGA coordinates based on prompt length
 * and the current buffer index to handle line-wrapping across 80 columns.
 */
static void redraw_line()
{
    /* Calculate final landing spot for hardware cursor after redraw */
    int final_total = PROMPT_LEN + cursor_idx;
    int final_x = final_total % width;
    int final_y = prompt_start_y + (final_total / width);

    /* Temporarily hide cursor to prevent ghosting during character output */
    hide_hardware_cursor();

    /* Reset software cursor to start of current input line */
    set_cursor(PROMPT_LEN, prompt_start_y);

    /* Overwrite current screen line(s) with updated buffer */
    for (int i = 0; i < buffer_len; i++)
    {
        putc(input_buffer[i]);
    }

    /* Print a trailing space to erase characters left over by backspaces */
    putc(' ');

    /* Synchronize VGA cursor registers with final calculated position */
    set_cursor(final_x, final_y);

    /* Restore hardware cursor visibility at the final destination */
    show_hardware_cursor();
}

// Command function prototypes
static void cmd_clear(const char *arg);
static void cmd_help(const char *arg);
static void cmd_mem(const char *arg);
static void cmd_tasks(const char *arg);
static void cmd_kill(const char *arg);
static void cmd_reboot(const char *arg);

// Command structure
typedef struct {
    const char *name;
    void (*func)(const char *arg);
    const char *help;
} command_t;

// Command array
static const command_t commands[] = {
    {"clear", cmd_clear, "Clear the terminal screen"},
    {"help", cmd_help, "Display this help menu"},
    {"mem", cmd_mem, "Show physical memory utilization"},
    {"tasks", cmd_tasks, "List running tasks"},
    {"kill", cmd_kill, "Kill a task (usage: kill <pid>)"},
    {"reboot", cmd_reboot, "Restart the system via PS/2"},
    {NULL, NULL, NULL} // Sentinel
};

/**
 * @brief Parse command and arguments
 */
static void parse_command(const char *input, char *cmd, char *arg) {
    const char *space = strchr(input, ' ');
    if (space) {
        size_t cmd_len = space - input;
        // Prevent buffer overflow
        if (cmd_len >= 32) cmd_len = 31;
        strncpy(cmd, input, cmd_len);
        cmd[cmd_len] = '\0';
        strcpy(arg, space + 1);
    } else {
        // Prevent buffer overflow
        if (strlen(input) >= 32) {
            strncpy(cmd, input, 31);
            cmd[31] = '\0';
        } else {
            strcpy(cmd, input);
        }
        arg[0] = '\0';
    }
}

/**
 * @brief Logic for interpreting and executing shell commands.
 * Uses a command table for cleaner dispatch and better maintainability.
 */
void process_command(char *cmd)
{
    char cmd_name[32];
    char cmd_arg[64];
    
    // Trim whitespace and parse
    parse_command(cmd, cmd_name, cmd_arg);
    
    // Handle empty command
    if (strlen(cmd_name) == 0) {
        return;
    }
    
    // Search for command in table
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(cmd_name, commands[i].name) == 0) {
            commands[i].func(cmd_arg);
            return;
        }
    }
    
    // Command not found
    printf("Error: '%s' is not recognized as a command.\n", cmd_name);
    printf("Type 'help' for available commands.\n");
}

// Command implementations
static void cmd_clear(const char *arg) {
    (void)arg; // Unused parameter
    print_clear();
}

static void cmd_help(const char *arg) {
    (void)arg; // Unused parameter
    puts("\n--- Valen Command Interface ---\n");
    for (int i = 0; commands[i].name != NULL; i++) {
        puts("  ");
        puts(commands[i].name);
        puts(" - ");
        puts(commands[i].help);
        puts("\n");
    }
    puts("----------------------------------\n");
}

static void cmd_mem(const char *arg) {
    (void)arg; // Unused parameter
    uint64_t total = pmm_get_total_kb();
    uint64_t used = pmm_get_used_kb();
    uint64_t free = total - used;

    puts("\n--- Physical Memory Mapping ---\n");
    printf("  Total: %llu MB\n", total / 1024);
    printf("  Used:  %llu MB\n", used / 1024);
    printf("  Free:  %llu MB\n", free / 1024);
    puts("-------------------------------\n");
}

static void cmd_tasks(const char *arg) {
    (void)arg; // Unused parameter
    puts("\n--- Running Tasks ---\n");
    
    task_t *current = get_current_task();
    if (!current) {
        puts("  No tasks running\n");
        puts("---------------------\n");
        return;
    }
    
    // Count and list all tasks
    int task_count = 0;
    task_t *task = current;
    
    do {
        const char *state_str = "UNKNOWN";
        switch (task->state) {
            case TASK_RUNNING: state_str = "RUNNING"; break;
            case TASK_INTERRUPTIBLE: state_str = "INTERRUPTIBLE"; break;
            case TASK_UNINTERRUPTIBLE: state_str = "UNINTERRUPTIBLE"; break;
            case TASK_ZOMBIE: state_str = "ZOMBIE"; break;
            case TASK_STOPPED: state_str = "STOPPED"; break;
            case TASK_TRACED: state_str = "TRACED"; break;
        }
        
        printf("  PID %d: %-16s (State: %s)\n", task->pid, task->comm, state_str);
        task_count++;
        task = task->next;
    } while (task != current && task != NULL);
    
    printf("  Total tasks: %d\n", task_count);
    puts("---------------------\n");
}

static void cmd_kill(const char *arg) {
    if (strlen(arg) == 0) {
        puts("Usage: kill <pid>\n");
        return;
    }
    
    int pid = atoi(arg);
    if (pid <= 0) {
        puts("Error: Invalid PID. PID must be a positive integer.\n");
        return;
    }
    
    int result = kill_task(pid);
    switch (result) {
        case 0:
            printf("Task with PID %d killed successfully.\n", pid);
            break;
        case -1:
            printf("Error: Task with PID %d not found.\n", pid);
            break;
        case -2:
            printf("Error: Cannot kill current shell task (PID %d).\n", pid);
            break;
        default:
            printf("Error: Unknown error killing task %d.\n", pid);
            break;
    }
}

static void cmd_reboot(const char *arg) {
    (void)arg; // Unused parameter
    puts("Sending reset signal to PS/2 controller...\n");
    outb(0x64, 0xFE);
}

/**
 * @brief Handles raw keyboard input characters for the shell.
 * This function is called by the keyboard interrupt handler to process
 * individual keystrokes and manage the input buffer and cursor position.
 */
void shell_input(signed char c)
{
    spinlock_acquire(&shell_lock);
    
    if (c == '\n')
    {
        input_buffer[buffer_len] = '\0';
        puts("\n");
        
        // Copy command to local buffer before releasing lock
        char cmd_copy[MAX_BUFFER];
        strcpy(cmd_copy, input_buffer);
        
        // Reset buffer state before releasing lock
        memset(input_buffer, 0, MAX_BUFFER);
        buffer_len = 0;
        cursor_idx = 0;
        
        spinlock_release(&shell_lock);
        
        process_command(cmd_copy);
        
        // Re-initialize shell for next command
        shell_init();
        return;
    }
    else if (c == '\b' && cursor_idx > 0)
    {
        // Handle backspace - shift characters left
        for (int i = cursor_idx - 1; i < buffer_len - 1; i++)
        {
            input_buffer[i] = input_buffer[i + 1];
        }
        buffer_len--;
        cursor_idx--;
        
        spinlock_release(&shell_lock);
        redraw_line();
    }
    else if (c == -1 && cursor_idx > 0)  // Left arrow
    {
        cursor_idx--;
        spinlock_release(&shell_lock);
        redraw_line();
    }
    else if (c == -2 && cursor_idx < buffer_len)  // Right arrow
    {
        cursor_idx++;
        spinlock_release(&shell_lock);
        redraw_line();
    }
    else if (c >= 32 && c <= 126 && buffer_len < MAX_BUFFER - 1)
    {
        // Insert character at cursor position
        for (int i = buffer_len; i > cursor_idx; i--)
        {
            input_buffer[i] = input_buffer[i - 1];
        }
        input_buffer[cursor_idx] = (char)c;
        buffer_len++;
        cursor_idx++;
        
        // Prevent buffer overflow
        if (buffer_len >= MAX_BUFFER) {
            buffer_len = MAX_BUFFER - 1;
            input_buffer[buffer_len] = '\0';
        }
        if (cursor_idx >= MAX_BUFFER) {
            cursor_idx = MAX_BUFFER - 1;
        }
        
        spinlock_release(&shell_lock);
        redraw_line();
    }
    else
    {
        spinlock_release(&shell_lock);
    }
}

/**
 * @brief Main shell task entry point
 * This function runs the shell as a task in the multitasking system
 */
void shell_task_main(void) {
    shell_init();
    
    while (1) {
        yield();
    }
}