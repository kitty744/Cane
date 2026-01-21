# Valen Changelog

## Version 1.0.0

- Basic shell interface with prompt "valen >> "
- Command processing system
- Basic input handling

## Version 1.0.1

### Fixed Issues:

- **Backspace not deleting first character**: Fixed cursor positioning logic to allow deletion of first character when cursor is at position 0
- **Arrow keys printing characters**: Fixed keyboard handler to prevent arrow keys (0x4B, 0x4D) from being processed through ASCII mapping
- **Character duplication on backspace/arrow keys**: Resolved cursor position conflicts between `putc()` and `redraw_line()` function
- **Extra space after prompt**: Fixed `PROMPT_LEN` from 8 to 9 to match actual length of "valen >> "
- **Invisible first character bug**: Resolved buffer management issues causing first character to not be recognized by shell
- **Cursor size inconsistency**: Added cursor size reset (14x15) in `shell_init()` to maintain consistent cursor appearance

### Improvements:

- Enhanced buffer management with proper null termination
- Improved `redraw_line()` function for better cursor handling
- Fixed keyboard input processing to prevent duplicate character handling
- Added proper line clearing to prevent display artifacts

### Technical Details:

- **Shell Buffer Management**: Fixed `input_buffer` handling with proper `buffer_len` and `cursor_idx` synchronization
- **Keyboard Driver**: Improved scancode processing to separate special keys from ASCII character mapping

_Note: Version 1.0.1 represents the stable shell implementation with all critical input/output bugs resolved._

## Version 1.1.0

### Major Features:

**Multitasking System:**

- Implemented complete task switching mechanism with process control blocks
- Added round-robin task scheduler with proper context switching
- Created x86_64 assembly context switch with register preservation
- Added process states (running, ready, blocked, terminated)
- Integrated shell as first system task
- Fixed stack allocation (3072 bytes) for optimal memory usage

**Timer System:**

- Implemented Programmable Interval Timer (PIT) driver
- Added hardware timer interrupt handler with proper EOI handling
- Created 10Hz timer interrupts for preemptive scheduling
- Integrated timer tick counter with task scheduler
- Fixed duplicate timer ISR definition causing system hangs

**Memory Management:**

- Fixed heap allocation with static 4KB heap area
- Resolved VMM memory mapping issues
- Improved task stack allocation and management

**Code Cleanup:**

- Removed all debug/temp/demo code
- Eliminated Linux references from comments
- Cleaned up interrupt handler implementations
- Optimized timer frequency for stable operation

### Technical Improvements:

- **Context Switch:** Fixed assembly register saving/restoring
- **Timer Interrupt:** Resolved C/assembly ISR conflicts
- **Heap System:** Replaced dynamic allocation with static heap
- **Task Creation:** Simplified and stabilized task initialization
- **Scheduler:** Improved first-task switching logic
