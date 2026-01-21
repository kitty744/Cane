# String Library

The String library provides essential string manipulation and memory operations for Valen kernel development.

## Overview

The String library implements basic string and memory functions needed for kernel operations. It provides memory-safe operations for string manipulation, copying, and comparison without requiring external dependencies.

## Quick Start

```c
#include <valen/string.h>

void kernel_main(void) {
    char buffer[256];

    // String operations
    strcpy(buffer, "Hello, Valen!");

    // Length operations
    int len = strlen(buffer);

    // Comparison
    if (strcmp(buffer, "expected") == 0) {
        // Strings match
    }

    // Memory operations
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, source, size);
}
```

## Available Functions

### String Operations

```c
// String length
int strlen(const char *str);

// String comparison
int strcmp(const char *str1, const char *str2);

// Partial string comparison
int strncmp(const char *str1, const char *str2, uint64_t n);

// Character search
char *strchr(const char *str, int c);

// String copy
char *strcpy(char *dest, const char *src);

// Length-limited string copy
char *strncpy(char *dest, const char *src, uint64_t n);
```

### Memory Operations

```c
// Memory fill
void *memset(void *ptr, int value, uint64_t num);

// Memory copy
void *memcpy(void *dest, const void *src, uint64_t num);
```

## Function Documentation

### strlen

Returns the length of a null-terminated string.

```c
int strlen(const char *str);
```

**Parameters:**

- `str` - Pointer to null-terminated string

**Returns:** Length of string (number of characters before null terminator)

**Example:**

```c
char *message = "Hello, World!";
int len = strlen(message);  // Returns 13
```

### strcmp

Compares two strings lexicographically.

```c
int strcmp(const char *str1, const char *str2);
```

**Parameters:**

- `str1` - First string to compare
- `str2` - Second string to compare

**Returns:**

- `< 0` if str1 < str2
- `> 0` if str1 > str2
- `0` if str1 == str2

**Example:**

```c
int result = strcmp("apple", "banana");  // Returns negative value
if (strcmp(str1, str2) == 0) {
    // Strings are equal
}
```

### strncmp

Compares first n characters of two strings lexicographically.

```c
int strncmp(const char *str1, const char *str2, uint64_t n);
```

**Parameters:**

- `str1` - First string to compare
- `str2` - Second string to compare
- `n` - Maximum number of characters to compare

**Returns:**

- `< 0` if str1 < str2
- `0` if str1 == str2 (for first n characters)
- `> 0` if str1 > str2

**Example:**

```c
int result = strncmp("hello world", "hello there", 5);  // Returns 0
int partial = strncmp("testing", "test", 4);          // Returns 0
```

### strchr

Finds first occurrence of a character in a string.

```c
char *strchr(const char *str, int c);
```

**Parameters:**

- `str` - String to search in
- `c` - Character to find

**Returns:** Pointer to first occurrence, or NULL if not found

**Example:**

```c
char *found = strchr("Hello, World", ',');    // Returns pointer to ','
char *first_a = strchr("banana", 'a');         // Returns pointer to first 'a'
```

### strcpy

Copies a source string to destination buffer.

```c
char *strcpy(char *dest, const char *src);
```

**Parameters:**

- `dest` - Destination buffer
- `src` - Source string

**Returns:** Pointer to destination string

**Example:**

```c
char dest[100];
strcpy(dest, "Hello, World!");
```

### strncpy

Copies up to n characters from source to destination buffer.

```c
char *strncpy(char *dest, const char *src, uint64_t n);
```

**Parameters:**

- `dest` - Destination buffer
- `src` - Source string
- `n` - Maximum characters to copy

**Returns:** Pointer to destination string

**Example:**

```c
char safe_dest[10];
strncpy(safe_dest, "This is a long string", 9);
safe_dest[9] = '\0';  // Ensure null termination
```

### memset

Fills a block of memory with a specified value.

```c
void *memset(void *ptr, int value, uint64_t num);
```

**Parameters:**

- `ptr` - Pointer to memory block
- `value` - Value to set (converted to unsigned char)
- `num` - Number of bytes to set

**Returns:** Pointer to the memory block

**Example:**

```c
char buffer[1024];
memset(buffer, 0, sizeof(buffer));  // Clear buffer
memset(buffer, 'A', 10);            // Fill first 10 bytes with 'A'
```

### memcpy

Copies a block of memory from source to destination.

```c
void *memcpy(void *dest, const void *src, uint64_t num);
```

**Parameters:**

- `dest` - Destination pointer
- `src` - Source pointer
- `num` - Number of bytes to copy

**Returns:** Pointer to destination

**Example:**

```c
char src[] = "Hello";
char dest[6];
memcpy(dest, src, sizeof(src));  // Copy "Hello" to dest
```

## Usage Examples

### String Processing

```c
void process_string(const char *input) {
    // Check if string is empty
    if (strlen(input) == 0) {
        printf("Empty string\n");
        return;
    }

    // Compare with known commands
    if (strcmp(input, "help") == 0) {
        show_help();
    } else if (strcmp(input, "status") == 0) {
        show_status();
    } else {
        printf("Unknown command: %s\n", input);
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

    // Use buffer for kernel operations
    process_data(buffer, sizeof(source));
}
```

### String Building

```c
void build_string(char *buffer, const char *name, int value) {
    // Copy name to buffer
    strcpy(buffer, name);

    // Add separator
    strcat(buffer, ": ");

    // Convert number to string and append
    char num_str[16];
    int_to_string(value, num_str);
    strcat(buffer, num_str);
}
```

## Implementation Details

### String Length Algorithm

```c
int strlen(const char *str) {
    int len = 0;
    while (*str++) {
        len++;
    }
    return len;
}
```

The function iterates through the string until it finds the null terminator, counting characters along the way.

### String Comparison Algorithm

```c
int strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(const unsigned char *)str1 - *(const unsigned char *)str2;
}
```

The function compares characters until it finds a difference or reaches the end of either string.

### Memory Operations

Both `memset` and `memcpy` use simple byte-by-byte loops:

```c
void *memset(void *ptr, int value, uint64_t num) {
    uint8_t *p = (uint8_t *)ptr;
    while (num--) {
        *p++ = (uint8_t)value;
    }
    return ptr;
}

void *memcpy(void *dest, const void *src, uint64_t num) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (num--) {
        *d++ = *s++;
    }
    return dest;
}
```

## Best Practices

1. **Buffer bounds checking** - Always verify buffer sizes before operations
2. **Null termination** - Ensure strings are properly null-terminated
3. **Memory alignment** - These functions work with any alignment
4. **Error handling** - Check return values where applicable
5. **Performance** - For large memory operations, consider specialized optimizations

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
    char *heap_buffer = malloc(512);
    if (heap_buffer) {
        strcpy(heap_buffer, "Kernel allocated string");
        // ... use heap_buffer ...
        free(heap_buffer);
    }
}
```

### Thread Safety

The string functions are not thread-safe by themselves. If used in multi-threaded contexts, provide appropriate synchronization:

```c
// For shared string operations
spinlock_acquire(&string_lock);
strcpy(shared_buffer, input);
spinlock_release(&string_lock);
```

## Integration Example

```c
#include <valen/string.h>
#include <valen/stdio.h>

void process_command(const char *command) {
    char buffer[128];

    // Copy command to buffer for processing
    memcpy(buffer, command, strlen(command) + 1);

    // Process different commands
    if (strcmp(buffer, "clear") == 0) {
        print_clear();
    } else if (strcmp(buffer, "help") == 0) {
        printf("Available commands: clear, help, status\n");
    } else if (strcmp(buffer, "status") == 0) {
        printf("System status: OK\n");
    } else {
        printf("Unknown command: %s\n", buffer);
    }
}

void memory_example(void) {
    // Allocate and initialize memory
    char *data = malloc(1024);
    if (data) {
        // Clear memory
        memset(data, 0, 1024);

        // Copy pattern
        char pattern[] = "Valen";
        memcpy(data, pattern, sizeof(pattern));

        // Use data...

        free(data);
    }
}
```

## Limitations

The current string library provides essential functions for kernel operations:

**Available Functions:**

**strlen** - String length calculation
**strcmp** - String comparison
**strncmp** - Partial string comparison
**strchr** - Character search
**strcpy** - String copy
**strncpy** - Length-limited string copy
**memset** - Memory fill
**memcpy** - Memory copy

**Not Yet Implemented:**

**strcasecmp** - Case-insensitive comparison
**strstr** - Substring search
**strtok** - String tokenization
**sprintf** - Formatted string functions
**strcat** - String concatenation

Additional functions can be added as needed for specific kernel requirements. The current implementation provides the essential functionality needed for basic kernel string and memory operations while remaining simple and efficient.

```

```
