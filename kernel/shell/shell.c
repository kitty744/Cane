/**
 * @file shell.c
 * @brief Valen Interactive Shell
 * * Provides a command-line interface for system interaction. This module
 * manages a local input buffer, handles character insertion/deletion,
 * and interfaces with the VGA driver to provide a flicker-free experience
 * through hardware cursor masking and coordinate math.
 */

#include <valen/stdio.h>
#include <valen/string.h>
#include <valen/io.h>
#include <valen/pmm.h>
#include <valen/heap.h>

#define MAX_BUFFER 256
#define PROMPT "valen >> "
#define PROMPT_LEN 9

static char input_buffer[MAX_BUFFER];
static int buffer_len = 0;
static int cursor_idx = 0;
static int prompt_start_y = 1;

/**
 * @brief Resets shell state and initializes prompt.
 * * Clears the internal input buffer, resets the logical cursor index,
 * and establishes the vertical anchor (prompt_start_y) for the current
 * command line.
 */
void shell_init()
{
    memset(input_buffer, 0, MAX_BUFFER);
    buffer_len = 0;
    cursor_idx = 0;

    if (get_cursor_y() < 1)
        set_cursor(0, 1);

    prompt_start_y = get_cursor_y();
    puts(PROMPT);

}

/**
 * @brief Redraws the command line while preventing cursor ghosting.
 * * This function masks the hardware cursor during the printing process.
 * It calculates the absolute VGA coordinates based on prompt length
 * and the current buffer index to handle line-wrapping across 80 columns.
 */
static void redraw_line()
{
    /* Calculate final landing spot for hardware cursor after redraw */
    int final_total = PROMPT_LEN + cursor_idx;
    int final_x = final_total % width;
    int final_y = prompt_start_y + (final_total / width);

    /* Temporarily hide cursor to prevent "ghosting" during kputc calls */
    hide_hardware_cursor();

    /* Reset software cursor to the start of the current input line */
    set_cursor(PROMPT_LEN, prompt_start_y);

    /* Overwrite the current screen line(s) with the updated buffer */
    for (int i = 0; i < buffer_len; i++)
    {
        putc(input_buffer[i]);
    }

    /* Print a trailing space to erase characters left over by backspaces */
    putc(' ');

    /* Synchronize the VGA cursor registers with the final calculated position */
    set_cursor(final_x, final_y);

    /* Restore hardware cursor visibility at the final destination */
    show_hardware_cursor();
}

/**
 * @brief Logic for interpreting and executing shell commands.
 * * Compares the input buffer against known commands and dispatches
 * to the appropriate kernel subsystems with formatted output.
 */
void process_command(char *cmd)
{
    if (strcmp(cmd, "clear") == 0)
    {
        print_clear();
    }
    else if (strcmp(cmd, "help") == 0)
    {
        puts("\n--- Valen Command Interface ---\n");
        puts("  help    - Display this menu\n");
        puts("  clear   - Clear the terminal screen\n");
        puts("  mem     - Show physical memory utilization\n");
        puts("  reboot  - Restart the system via PS/2\n");
        puts("----------------------------------\n");
    }
    else if (strcmp(cmd, "mem") == 0)
    {
        uint64_t total = pmm_get_total_kb();
        uint64_t used = pmm_get_used_kb();
        uint64_t free = total - used;

        puts("\n--- Physical Memory Mapping ---\n");
        puts("  Total: ");
        print_int(total / 1024);
        puts(" MB\n");
        puts("  Used:  ");
        print_int(used / 1024);
        puts(" MB\n");
        puts("  Free:  ");
        print_int(free / 1024);
        puts(" MB\n");
        puts("-------------------------------\n");
    }
    else if (strcmp(cmd, "reboot") == 0)
    {
        puts("Sending reset signal to PS/2 controller...\n");
        outb(0x64, 0xFE);
    }
    else if (strlen(cmd) > 0)
    {
        puts("Error: '");
        puts(cmd);
        puts("' is not recognized as a command.\n");
    }
}

/**
 * @brief Handles raw keyboard input characters.
 */
void shell_input(signed char c)
{
    if (c == '\n')
    {
        input_buffer[buffer_len] = '\0';
        puts("\n");
        process_command(input_buffer);
        shell_init();
    }
    else if (c == '\b' && cursor_idx > 0)
    {
        for (int i = cursor_idx - 1; i < buffer_len - 1; i++)
        {
            input_buffer[i] = input_buffer[i + 1];
        }
        buffer_len--;
        cursor_idx--;
        
        redraw_line();
    }
    else if (c == -1 && cursor_idx > 0)
    {
        cursor_idx--;
        redraw_line();
    }
    else if (c == -2 && cursor_idx < buffer_len)
    {
        cursor_idx++;
        redraw_line();
    }
    else if (c >= 32 && c <= 126 && buffer_len < MAX_BUFFER - 1)
    {
        /* Inserting in middle - need to redraw */
        for (int i = buffer_len; i > cursor_idx; i--)
        {
            input_buffer[i] = input_buffer[i - 1];
        }
        input_buffer[cursor_idx] = (char)c;
        buffer_len++;
        cursor_idx++;
        redraw_line();
    }
}