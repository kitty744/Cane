#include <valen/pic.h>
#include <valen/io.h>
#include <valen/spinlock.h>

static spinlock_t pic_lock = SPINLOCK_INIT;

/* Helper functions to wait for PIC command completion */
static void pic_wait_command(uint16_t port)
{
    /* Small delay to allow PIC to process command */
    for (int i = 0; i < 1000; i++) {
        asm volatile("nop");
    }
}

/**
 * @brief Remaps PIC interrupt vectors to avoid conflicts with CPU exceptions.
 * 
 * The default PIC mappings (0x08-0x0F and 0x70-0x77) conflict with
 * CPU exceptions. We remap them to 0x20-0x2F.
 */
void pic_remap(uint8_t offset1, uint8_t offset2)
{
    spinlock_acquire(&pic_lock);
    
    /* Save current interrupt masks */
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);
    
    /* Start initialization sequence */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    pic_wait_command(PIC1_COMMAND);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    pic_wait_command(PIC2_COMMAND);
    
    /* Set vector offsets */
    outb(PIC1_DATA, offset1);
    pic_wait_command(PIC1_DATA);
    outb(PIC2_DATA, offset2);
    pic_wait_command(PIC2_DATA);
    
    /* Configure cascade */
    outb(PIC1_DATA, 4);    /* Master PIC: IRQ2 is connected to slave */
    pic_wait_command(PIC1_DATA);
    outb(PIC2_DATA, 2);    /* Slave PIC: cascade identity */
    pic_wait_command(PIC2_DATA);
    
    /* Set 8086 mode */
    outb(PIC1_DATA, ICW4_8086);
    pic_wait_command(PIC1_DATA);
    outb(PIC2_DATA, ICW4_8086);
    pic_wait_command(PIC2_DATA);
    
    /* Restore interrupt masks */
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
    
    spinlock_release(&pic_lock);
}

/**
 * @brief Initialize the PIC with proper remapping.
 */
void pic_init(void)
{
    /* Remap PIC interrupts to avoid conflicts with CPU exceptions */
    pic_remap(PIC1_VECTOR_OFFSET, PIC2_VECTOR_OFFSET);
    
    /* Disable all interrupts initially */
    pic_irq_mask_all();
}

/**
 * @brief Send End of Interrupt (EOI) signal to PIC.
 * @param irq The IRQ line to acknowledge (0-15)
 */
void pic_send_eoi(uint8_t irq)
{
    spinlock_acquire(&pic_lock);
    
    if (irq >= 8) {
        /* Send EOI to slave PIC */
        outb(PIC2_COMMAND, PIC_EOI);
    }
    
    /* Always send EOI to master PIC */
    outb(PIC1_COMMAND, PIC_EOI);
    
    spinlock_release(&pic_lock);
}

/**
 * @brief Enable specific IRQ line.
 * @param irq The IRQ line to enable (0-15)
 */
void pic_irq_enable(uint8_t irq)
{
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    spinlock_acquire(&pic_lock);
    
    value = inb(port) & ~(1 << irq);
    outb(port, value);
    
    spinlock_release(&pic_lock);
}

/**
 * @brief Disable specific IRQ line.
 * @param irq The IRQ line to disable (0-15)
 */
void pic_irq_disable(uint8_t irq)
{
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    spinlock_acquire(&pic_lock);
    
    value = inb(port) | (1 << irq);
    outb(port, value);
    
    spinlock_release(&pic_lock);
}

/**
 * @brief Mask (disable) all IRQ lines.
 */
void pic_irq_mask_all(void)
{
    spinlock_acquire(&pic_lock);
    
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
    
    spinlock_release(&pic_lock);
}

/**
 * @brief Unmask (enable) all IRQ lines.
 */
void pic_irq_unmask_all(void)
{
    spinlock_acquire(&pic_lock);
    
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
    
    spinlock_release(&pic_lock);
}

/**
 * @brief Get Interrupt Request Register (IRR) status.
 * @return 16-bit value with IRQ status bits
 */
uint16_t pic_get_irr(void)
{
    spinlock_acquire(&pic_lock);
    
    outb(PIC1_COMMAND, 0x0A);    /* Read IRR command */
    uint16_t irr = inb(PIC1_COMMAND);
    
    outb(PIC2_COMMAND, 0x0A);    /* Read IRR command */
    irr |= (inb(PIC2_COMMAND) << 8);
    
    spinlock_release(&pic_lock);
    
    return irr;
}

/**
 * @brief Get In-Service Register (ISR) status.
 * @return 16-bit value with IRQ status bits
 */
uint16_t pic_get_isr(void)
{
    spinlock_acquire(&pic_lock);
    
    outb(PIC1_COMMAND, 0x0B);    /* Read ISR command */
    uint16_t isr = inb(PIC1_COMMAND);
    
    outb(PIC2_COMMAND, 0x0B);    /* Read ISR command */
    isr |= (inb(PIC2_COMMAND) << 8);
    
    spinlock_release(&pic_lock);
    
    return isr;
}
