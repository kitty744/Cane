# Timer System

## Overview

The Valen timer system provides hardware-based timing capabilities using the Programmable Interval Timer (PIT). It drives the preemptive multitasking scheduler and provides timing services for the kernel.

## Components

### Programmable Interval Timer (PIT)

The Intel 8253/8254 PIT provides three timing channels:

- **Channel 0**: System timer (connected to IRQ 0)
- **Channel 1**: (Unused - typically DRAM refresh)
- **Channel 2**: PC speaker (unused)

### Timer Configuration

```c
void pit_init(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;

    // Configure Channel 0, square wave mode
    outb(PIT_COMMAND_PORT, 0x36);
    outb(PIT_DATA_PORT_0, divisor & 0xFF);        // Low byte
    outb(PIT_DATA_PORT_0, (divisor >> 8) & 0xFF);   // High byte

    // Enable timer IRQ
    pic_irq_enable(0);
}
```

## Timer Modes

### Square Wave Mode (Mode 3)

- **Configuration**: 0x36 command byte
- **Behavior**: Generates periodic square wave
- **Usage**: System timer for scheduling
- **Frequency**: Configurable (default: 10Hz)

### Frequency Calculation

```
divisor = 1193180 / desired_frequency
```

- **Base Frequency**: 1.19318 MHz
- **Maximum Period**: ~55ms (18.2Hz minimum)
- **Minimum Period**: ~838ns (1.193MHz maximum)

## Interrupt Handling

### Timer Interrupt Service Routine

```assembly
timer_isr:
    ; Save registers
    push rax
    push rcx
    push rdx
    ; ... (other registers)

    ; Call scheduler tick
    call scheduler_tick

    ; Send EOI to PIC
    mov rdi, 0
    call pic_send_eoi

    ; Restore registers
    pop rdx
    pop rcx
    pop rax
    iretq
```

### Scheduler Integration

```c
void scheduler_tick(void) {
    if (!current_task) return;

    static int counter = 0;
    counter++;

    // Schedule every 10 ticks (1 second at 10Hz)
    if (counter >= 10) {
        counter = 0;
        schedule();
    }
}
```

## Configuration

### Current Settings

- **Frequency**: 10Hz (100ms intervals)
- **Scheduling**: Every 10 ticks (1 second)
- **IRQ**: 0 (mapped to vector 0x20)
- **Mode**: Square wave (Mode 3)

### Adjusting Timer Frequency

```c
// For faster scheduling (100Hz)
pit_init(100);

// For slower scheduling (1Hz)
pit_init(1);
```

## API Reference

### Timer Initialization

```c
void pit_init(uint32_t frequency);
```

- **Purpose**: Initialize PIT with specified frequency
- **Parameter**: `frequency` - Desired frequency in Hz
- **Effect**: Configures Channel 0 and enables IRQ 0

### Timer Tick Handler

```c
void scheduler_tick(void);
```

- **Purpose**: Called by timer interrupt handler
- **Behavior**: Increments counter, triggers scheduler
- **Frequency**: Called at PIT frequency

## Hardware Details

### I/O Ports

- **0x43**: PIT command port
- **0x40**: Channel 0 data port

### Command Byte Format

```
Bit 7-6: Channel select (00 = Channel 0)
Bit 5-4: Access mode (11 = lobyte/hibyte)
Bit 3-1: Operating mode (011 = square wave)
Bit 0:   BCD mode (0 = binary)
```

### PIC Integration

- **IRQ Line**: 0 (timer)
- **Vector**: 0x20 (after PIC remapping)
- **EOI**: Required after each interrupt

## Performance Considerations

### Timer Frequency Impact

- **Higher Frequency**: More responsive scheduling, higher overhead
- **Lower Frequency**: Less overhead, slower task switching
- **Current Setting**: 10Hz balances responsiveness and efficiency

### Interrupt Overhead

- **Context Switch**: ~10-20 microseconds
- **Register Save/Restore**: ~5 microseconds
- **Scheduler Logic**: ~1-2 microseconds

## Troubleshooting

### Common Issues

1. **Timer Not Firing**: Check PIC initialization and IRQ enable
2. **System Hangs**: Verify EOI is sent to PIC
3. **Frequency Issues**: Validate divisor calculation
4. **Interrupt Storm**: Check for duplicate ISR definitions

### Debug Information

Timer interrupts can be debugged by adding prints to `scheduler_tick()` or checking the PIC interrupt status registers.

## Future Enhancements

- **APIC Timer**: Modern replacement for PIT
- **High Resolution Timers**: TSC/HPET integration
- **One-Shot Mode**: For precise timing
- **Multiple Timers**: For different scheduling needs
