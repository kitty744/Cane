## Printf Formatting Options

CaneOS includes an enhanced printf with extensive formatting:

```c
// Numbers
printf("%d", -42);        // Signed integer
printf("%u", 42);         // Unsigned integer
printf("%ld", 12345678);  // Long integer
printf("%llu", 1234567890); // Long long unsigned

// Bases
printf("%x", 255);        // Hexadecimal (lowercase)
printf("%X", 255);        // Hexadecimal (uppercase)
printf("%o", 8);          // Octal
printf("%b", 5);          // Binary

// Other
printf("%p", ptr);        // Pointer (0x format)
printf("%s", "hello");    // String
printf("%c", 'A');        // Character
printf("%%");             // Literal percent
```
