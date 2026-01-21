#ifndef IO_H
#define IO_H

#include <stdint.h>

/**
 * @brief Reads a byte (8 bits) from the specified I/O port.
 * @param port The I/O port address.
 * @return The byte read from the port.
 */
static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    /* 'a' (al) is the destination, 'Nd' allows for 8-bit immediate port or dx register */
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/**
 * @brief Writes a byte (8 bits) to the specified I/O port.
 * @param port The I/O port address.
 * @param val The byte value to write.
 */
static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/**
 * @brief Reads a word (16 bits) from the specified I/O port.
 * Required for ATA PIO data transfers.
 * @param port The I/O port address.
 * @return The 16-bit value read from the port.
 */
static inline uint16_t inw(uint16_t port)
{
    uint16_t ret;
    /* 'a' (ax) is the destination */
    asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/**
 * @brief Writes a word (16 bits) to the specified I/O port.
 * Required for ATA PIO data transfers.
 * @param port The I/O port address.
 * @param val The 16-bit value to write.
 */
static inline void outw(uint16_t port, uint16_t val)
{
    asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

/**
 * @brief Reads a double word (32 bits) from the specified I/O port.
 * Required for PCI Configuration Space data access.
 * @param port The I/O port address.
 * @return The 32-bit value read from the port.
 */
static inline uint32_t inl(uint16_t port)
{
    uint32_t ret;
    /* 'a' (eax) is the destination */
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/**
 * @brief Writes a double word (32 bits) to the specified I/O port.
 * Required for PCI Configuration Space address selection.
 * @param port The I/O port address.
 * @param val The 32-bit value to write.
 */
static inline void outl(uint16_t port, uint32_t val)
{
    asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

#endif