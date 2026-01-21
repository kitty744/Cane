#include <valen/spinlock.h>

void spinlock_init(spinlock_t *lock)
{
    lock->lock = 0;
}

void spinlock_acquire(spinlock_t *lock)
{
    while (1) {
        uint32_t expected = 0;
        uint32_t desired = 1;
        
        asm volatile (
            "lock cmpxchgl %2, %1"
            : "+a" (expected), "+m" (lock->lock)
            : "r" (desired)
            : "memory", "cc"
        );
        
        if (expected == 0) {
            break;
        }
        
        asm volatile ("pause");
    }
}

void spinlock_release(spinlock_t *lock)
{
    asm volatile (
        "movl $0, %0"
        : "=m" (lock->lock)
        :
        : "memory"
    );
}

uint8_t spinlock_try_acquire(spinlock_t *lock)
{
    uint32_t expected = 0;
    uint32_t desired = 1;
    
    asm volatile (
        "lock cmpxchgl %2, %1"
        : "+a" (expected), "+m" (lock->lock)
        : "r" (desired)
        : "memory", "cc"
    );
    
    return (expected == 0);
}