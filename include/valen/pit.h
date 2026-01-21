#ifndef PIT_H
#define PIT_H

#include <stdint.h>

/**
 * @brief Initialize the PIT with specified frequency
 * @param frequency The desired timer frequency in Hz
 */
void pit_init(uint32_t frequency);

#endif
