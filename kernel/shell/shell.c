/**
 * @file shell.c
 * @brief CaneOS Interactive Shell
 * * Provides a command-line interface for system interaction. This module
 * manages a local input buffer, handles character insertion/deletion,
 * and interfaces with the VGA driver to provide a flicker-free experience
 * through hardware cursor masking and coordinate math.
 */

#include "cane/stdio.h"
#include "cane/string.h"
#include "cane/io.h"
#include "cane/pmm.h"
#include "cane/heap.h"

#define MAX_BUFFER 256
#define PROMPT "Nexus >> "
#define PROMPT_LEN 9

static char input_buffer[MAX_BUFFER];
static int buffer_len = 0;
static int cursor_idx = 0;
static int prompt_start_y = 1;

/**
 * @brief Disables the hardware cursor rendering.
 * * Communicates with the CRT Controller (CRTC) registers. Setting bit 5
 * of the Cursor Start Register (0x0A) instructs the VGA hardware to
 * stop rendering the blinking cursor.
 */
static void hide_hardware_cursor()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, inb(0x3D5) | 0x20);
}

/**
 * @brief Enables the hardware cursor rendering.
 * * Clears bit 5 of the Cursor Start Register (0x0A) to allow the
 * VGA hardware to render the blinking cursor at the current register position.
 */
static void show_hardware_cursor()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, inb(0x3D5) & ~0x20);
}

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

    /* Ensure the shell starts below the row 0 status bar */
    if (get_cursor_y() < 1)
        set_cursor(0, 1);

    prompt_start_y = get_cursor_y();
    printf(PROMPT);

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

    /* Temporarily hide cursor to prevent "ghosting" during putc calls */
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
        printf("\n--- CaneOS Command Interface ---\n");
        printf("  help    - Display this menu\n");
        printf("  clear   - Clear the terminal screen\n");
        printf("  mem     - Show physical memory utilization\n");
        printf("----------------------------------\n");
    }
    else if (strcmp(cmd, "mem") == 0)
    {
        uint64_t total = pmm_get_total_kb();
        uint64_t used = pmm_get_used_kb();
        uint64_t free = total - used;

        printf("\n--- Physical Memory Mapping ---\n");
        printf("  Total: ");
        print_int(total / 1024);
        printf(" MB\n");
        printf("  Used:  ");
        print_int(used / 1024);
        printf(" MB\n");
        printf("  Free:  ");
        print_int(free / 1024);
        printf(" MB\n");
        printf("-------------------------------\n");
    }
    else if (strcmp(cmd, "reboot") == 0)
    {
        printf("Sending reset signal to PS/2 controller...\n");
        outb(0x64, 0xFE);
    }
    else if (strlen(cmd) > 0)
    {
        printf("Error: '");
        printf(cmd);
        printf("' is not recognized as a command.\n");
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
        printf("\n");
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