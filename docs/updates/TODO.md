# ValenOS TODO List

## Version 1.1.0 - Core System Features

### Multitasking

- [ ] Implement basic task switching mechanism
- [ ] Add process control block (PCB) structure
- [ ] Create task scheduler with round-robin algorithm
- [ ] Implement context switching for x86_64
- [ ] Add process states (running, ready, blocked, terminated)
- [ ] Create system calls for process management

### Timer System

- [ ] Initialize hardware timer (PIT)
- [ ] Implement timer interrupt handler
- [ ] Add timer tick counter for scheduling
- [ ] Create sleep/delay functions
- [ ] Integrate timer with task scheduler

### Memory Management

- [ ] Enhanced heap allocation with fragmentation handling
- [ ] Implement garbage collection for unused memory
- [ ] Add memory protection mechanisms
- [ ] Create memory mapping for processes
- [ ] Implement virtual memory management improvements

## Version 1.1.0 - Driver Improvements

### Enhanced Keyboard Support

- [ ] Extend scancode mapping for function keys (F1-F12)
- [ ] Add numpad support
- [ ] Implement key repeat rate handling
- [ ] Add international keyboard layouts
- [ ] Create keyboard event queue system

### Mouse Driver

- [ ] Initialize PS/2 mouse controller
- [ ] Handle mouse packet parsing
- [ ] Implement cursor movement tracking
- [ ] Add mouse button event handling
- [ ] Create mouse driver interface

### Storage Drivers

- [ ] Basic ATA disk driver implementation
- [ ] Add SATA controller support
- [ ] Implement disk read/write operations
- [ ] Create file system interface
- [ ] Add partition table parsing

## Version 1.1.0 - Shell Enhancements

### Command History

- [ ] Create command history buffer
- [ ] Add up/down arrow navigation
- [ ] Implement history persistence across reboots
- [ ] Add history size limiting
- [ ] Create history search functionality

### Tab Completion

- [ ] Implement filename completion
- [ ] Add command name completion
- [ ] Create completion matching algorithm
- [ ] Handle ambiguous completions
- [ ] Add completion cycling with tab

### Pipeline Support

- [ ] Implement pipe operator (|) parsing
- [ ] Create process chaining mechanism
- [ ] Add input/output redirection
- [ ] Handle background processes (&)
- [ ] Implement signal handling

### Environment Variables

- [ ] Create environment variable storage
- [ ] Add PATH variable support
- [ ] Implement HOME directory handling
- [ ] Add variable expansion
- [ ] Create export/unset commands

## Notes

- Prioritize Version 1.1.0 features for next development cycle
- Focus on stable multitasking foundation before adding complex features
- Test each driver implementation thoroughly before integration
- Maintain shell compatibility during system upgrades
