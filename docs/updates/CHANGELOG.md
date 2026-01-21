# ValenOS Changelog

## Version 1.0.0 - Initial Shell Implementation

- Basic shell interface with prompt "Valen >> "
- Command processing system
- Basic input handling

## Version 1.0.1 - Shell Fixes & Improvements

### Fixed Issues:

- **Backspace not deleting first character**: Fixed cursor positioning logic to allow deletion of first character when cursor is at position 0
- **Arrow keys printing characters**: Fixed keyboard handler to prevent arrow keys (0x4B, 0x4D) from being processed through ASCII mapping
- **Character duplication on backspace/arrow keys**: Resolved cursor position conflicts between `putc()` and `redraw_line()` function
- **Extra space after prompt**: Fixed `PROMPT_LEN` from 9 to 8 to match actual length of "Valen >> "
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
