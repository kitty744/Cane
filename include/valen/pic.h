#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/* PIC 8259 chip I/O ports */
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1

/* PIC commands */
#define PIC_EOI         0x20    /* End of interrupt */
#define PIC_INIT        0x11    /* Initialize */
#define PIC_ICW4_8086   0x01    /* 8086 mode */

/* PIC initialization control words */
#define ICW1_ICW4       0x01    /* ICW4 (not) needed */
#define ICW1_SINGLE     0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04    /* Call address interval 4 (8) */
#define ICW1_LEVEL      0x08    /* Level triggered (edge) mode */
#define ICW1_INIT       0x10    /* Initialization - required! */

#define ICW4_8086       0x01    /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02    /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08    /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C    /* Buffered mode/master */
#define ICW4_SFNM       0x10    /* Special fully nested (not) */

/* Default IRQ mappings before remapping */
#define PIC1_VECTOR_BASE    0x08    /* Master PIC: IRQ 0-7 -> 0x08-0x0F */
#define PIC2_VECTOR_BASE    0x70    /* Slave PIC: IRQ 8-15 -> 0x70-0x77 */

/* New IRQ mappings after remapping */
#define PIC1_VECTOR_OFFSET  0x20    /* Master PIC: IRQ 0-7 -> 0x20-0x27 */
#define PIC2_VECTOR_OFFSET  0x28    /* Slave PIC: IRQ 8-15 -> 0x28-0x2F */

/* IRQ lines */
#define IRQ_TIMER       0
#define IRQ_KEYBOARD    1
#define IRQ_CASCADE     2    /* Used internally by slave PIC */
#define IRQ_COM2        3
#define IRQ_COM1        4
#define IRQ_LPT2        5
#define IRQ_FLOPPY      6
#define IRQ_LPT1        7
#define IRQ_RTC         8
#define IRQ_FREE1       9
#define IRQ_FREE2       10
#define IRQ_FREE3       11
#define IRQ_MOUSE       12
#define IRQ_FPU         13
#define IRQ_ATA1        14
#define IRQ_ATA2        15

void pic_init(void);
void pic_remap(uint8_t offset1, uint8_t offset2);
void pic_send_eoi(uint8_t irq);
void pic_irq_enable(uint8_t irq);
void pic_irq_disable(uint8_t irq);
void pic_irq_mask_all(void);
void pic_irq_unmask_all(void);
uint16_t pic_get_irr(void);
uint16_t pic_get_isr(void);

#endif