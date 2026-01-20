# String Library

The String library provides essential string manipulation and memory operations for CaneOS kernel development.

## Overview

The String library implements common string functions similar to the C standard library, optimized for kernel usage. It provides memory-safe operations for string manipulation, copying, and comparison without requiring external dependencies.

## Quick Start

```c
#include "cane/string.h"

void kernel_main(void) {
    char buffer[256];

    // String operations
    strcpy(buffer, "Hello, CaneOS!");
    strcat(buffer, " Welcome to the kernel.");

    // Length operations
    size_t len = strlen(buffer);

    // Comparison
    if (strcmp(buffer, "expected") == 0) {
        // Strings match
    }

    // Memory operations
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, source, size);
}
```

## String Operations

### Basic String Functions

```c
// String length
size_t strlen(const char *str);

// String copy
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);

// String concatenation
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);

// String comparison
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
```

### String Search Functions

```c
// Find character in string
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);

// Find substring
char *strstr(const char *haystack, const char *needle);

// Find first/last occurrence of any character from set
size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
char *strpbrk(const char *s, const char *accept);
```

### String Tokenization

```c
// Split string into tokens
char *strtok(char *str, const char *delim);
char *strtok_r(char *str, const char *delim, char **saveptr);
```

## Memory Operations

### Basic Memory Functions

```c
// Memory copy
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);

// Memory fill
void *memset(void *s, int c, size_t n);

// Memory comparison
int memcmp(const void *s1, const void *s2, size_t n);
```

### Memory Search

```c
// Find byte in memory
void *memchr(const void *s, int c, size_t n);
void *memrchr(const void *s, int c, size_t n);
```

## Usage Examples

### Safe String Handling

```c
void safe_string_example(void) {
    char buffer[64];
    const char *input = "This is a long string that might be truncated";

    // Safe copy with length limit
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';  // Ensure null termination

    // Safe concatenation
    if (strlen(buffer) + strlen(" more") < sizeof(buffer)) {
        strcat(buffer, " more");
    }
}
```

### String Parsing

```c
void parse_command_line(char *cmdline) {
    char *token;
    char *saveptr;

    // Parse command arguments
    token = strtok_r(cmdline, " \t", &saveptr);
    while (token != NULL) {
        // Process each token
        printf("Argument: %s\n", token);
        token = strtok_r(NULL, " \t", &saveptr);
    }
}
```

### Memory Buffer Operations

```c
void buffer_operations(void) {
    uint8_t buffer[1024];

    // Clear buffer
    memset(buffer, 0, sizeof(buffer));

    // Copy data
    uint8_t source[] = {0x01, 0x02, 0x03, 0x04};
    memcpy(buffer, source, sizeof(source));

    // Find specific byte
    uint8_t *found = memchr(buffer, 0x03, sizeof(buffer));
    if (found != NULL) {
        printf("Found byte at offset: %zu\n", found - buffer);
    }
}
```

### String Validation

```c
bool is_valid_filename(const char *filename) {
    // Check for empty string
    if (strlen(filename) == 0) {
        return false;
    }

    // Check for invalid characters
    const char *invalid_chars = "/\\:*?\"<>|";
    if (strpbrk(filename, invalid_chars) != NULL) {
        return false;
    }

    // Check length
    if (strlen(filename) > 255) {
        return false;
    }

    return true;
}
```

## Advanced Usage

### Custom String Functions

```c
// Convert string to lowercase
void strlower(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] + 32;
        }
    }
}

// Trim whitespace from string
char *strtrim(char *str) {
    char *end;

    // Trim leading space
    while (*str == ' ' || *str == '\t' || *str == '\n') str++;

    if (*str == 0) return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n')) end--;

    end[1] = '\0';
    return str;
}
```

### Format String Validation

```c
bool safe_printf_format(const char *format) {
    // Check for format string vulnerabilities
    const char *p = format;

    while (*p) {
        if (*p == '%') {
            p++;
            if (*p == '%') {
                p++;
                continue;
            }

            // Skip format specifiers
            while (*p && *p != ' ' && *p != '\n' && *p != '\t') {
                if (strchr("diuoxXfFeEgGaAcspn%", *p)) {
                    break;
                }
                p++;
            }
        }
        p++;
    }

    return true;
}
```

## Best Practices

1. **Buffer bounds checking** - Always verify buffer sizes before operations
2. **Null termination** - Ensure strings are properly null-terminated
3. **Use safe variants** - Prefer strncpy/strncat over strcpy/strcat
4. **Memory initialization** - Initialize buffers with memset when needed
5. **Error handling** - Check return values and handle edge cases

## Performance Considerations

- **String length caching** - Cache strlen results for repeated use
- **Memory alignment** - Ensure proper alignment for memory operations
- **Avoid unnecessary copies** - Use pointers when possible instead of copying
- **Batch operations** - Combine multiple small operations when possible

## Kernel-Specific Considerations

### No Dynamic Allocation

All string operations work with pre-allocated buffers:

```c
void kernel_string_example(void) {
    // Stack-allocated buffers
    char stack_buffer[256];

    // Static buffers
    static char static_buffer[1024];

    // Use with kernel memory management
    char *heap_buffer = kmalloc(512);
    if (heap_buffer) {
        strcpy(heap_buffer, "Kernel allocated string");
        // ... use heap_buffer ...
        kfree(heap_buffer);
    }
}
```

### Interrupt Safety

String functions are not interrupt-safe by default:

```c
// For interrupt contexts, use atomic operations
void interrupt_safe_string_copy(volatile char *dest, const volatile char *src, size_t n) {
    // Disable interrupts during operation
    disable_interrupts();
    memcpy((void*)dest, (const void*)src, n);
    enable_interrupts();
}
```

## Integration Example

```c
#include "cane/string.h"

void process_user_input(const char *input) {
    char buffer[128];
    char *args[16];
    int arg_count = 0;

    // Copy and normalize input
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    strtrim(buffer);
    strlower(buffer);

    // Parse arguments
    char *token = strtok(buffer, " \t");
    while (token != NULL && arg_count < 16) {
        args[arg_count++] = token;
        token = strtok(NULL, " \t");
    }

    // Process command
    if (arg_count > 0) {
        if (strcmp(args[0], "help") == 0) {
            show_help();
        } else if (strcmp(args[0], "status") == 0) {
            show_system_status();
        }
    }
}
```

## Error Handling

String functions return specific values for error conditions:

- **NULL pointers** - Most functions return NULL or -1 for invalid inputs
- **Buffer overflows** - Safe variants prevent overflows but may truncate
- **Invalid characters** - Search functions return NULL when not found
- **Empty strings** - Length functions return 0, comparison may succeed

Always check return values and handle edge cases appropriately in kernel code.
