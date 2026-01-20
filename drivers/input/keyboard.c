/**
 * @file keyboard.c
 * @brief PS/2 Keyboard Driver for CaneOS.
 */

#include <cane/keyboard.h>
#include <cane/io.h>
#include <cane/shell.h>
#include <cane/pic.h>

extern int system_ready;

volatile int key_pressed_flag;

static int shift_pressed = 0;

/* Standard US-QWERTY Scancode Mapping */
static const char scancode_to_ascii[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '};

static const char scancode_to_ascii_shift[] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '};

void keyboard_init(void)
{
    while (inb(0x64) & 1)
        inb(0x60);
    
    /* Enable keyboard IRQ */
    pic_irq_enable(IRQ_KEYBOARD);
}

void wait_for_keypress(void)
{
    key_pressed_flag = 0;
    while (!key_pressed_flag)
    {
        /* Keep interrupts enabled so the IRQ can set the flag */
        asm volatile("hlt");
    }
}


/**
 * @brief Primary PS/2 IRQ1 Handler.
 */
void keyboard_handler()
{
    uint8_t status = inb(0x64);

    if ((status & 0x01) && !(status & 0x20)) {
        uint8_t scancode = inb(0x60);

        if (scancode == 0x2A || scancode == 0x36)
            shift_pressed = 1;
        else if (scancode == 0xAA || scancode == 0xB6)
            shift_pressed = 0;
        else if (!(scancode & 0x80)) {
            key_pressed_flag = 1;
            if (system_ready) {
                switch (scancode) {
                case 0x0E:
                    shell_input('\b');
                    break;
                case 0x1C:
                    shell_input('\n');
                    break;
                case 0x4B:
                    shell_input(-1);
                    break;
                case 0x4D:
                    shell_input(-2);
                    break;
                default:
                    /* Only process ASCII characters for valid scancodes */
                    if (scancode < sizeof(scancode_to_ascii)) {
                        char c = shift_pressed ? scancode_to_ascii_shift[scancode] : scancode_to_ascii[scancode];
                        if (c && c != '\0') {
                            shell_input(c);
                        }
                    }
                    break;
                }
            }
        }
    }

    pic_send_eoi(IRQ_KEYBOARD);
}
