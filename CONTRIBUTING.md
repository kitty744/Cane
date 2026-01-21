# Contributing to ValenOS

Thank you for your interest in ValenOS! Developing a kernel requires strict adherence to memory safety and code style to prevent system-wide instability.

## 1. Code Style

- **Naming:** All core kernel functions should be lowercase and descriptive.
- **Headers:** Always include the relevant `<Valen/xxx.h>` headers.
- **Indentation:** Use 4 spaces for indentation (no tabs).

## 2. Memory Management (The Golden Rules)

ValenOS uses a custom heap and VMM. To keep the system stable:

- **NULL Checks:** Every `malloc()` or `vmm_alloc()` call **MUST** be followed by a NULL check.
- **Locking:** If you acquire the `heap_lock` or any `spinlock_t`, you must release it on every possible exit path of the function.
- **Magic Numbers:** Do not manually modify the `magic` field in `heap_node_t`.

## 3. Static Analysis

Before submitting a pull request, you must ensure your code passes our Coccinelle suite. This catches deadlocks and memory leaks that the compiler misses.

Run the following command:

```bash
make coccinelle
```

If any output is produced, fix the flagged lines before committing.

## 4. Development Workflow

1.  **Read** the documentation.
2.  **Fork** the repository.
3.  **Create a branch** for your feature (e.g., `git checkout -b feature/driver-x`).
4.  **Write tests** if applicable.
5.  **Run the analysis** (`make coccinelle`).
6.  **Add your name** to CREDITS
7.  **Add your email** to .mailmap
8.  **Submit a Pull Request** with a detailed description of your changes.

## 5. Panic and Debugging

If your changes trigger a **Fatal Page Fault**, use the error code and fault address printed by `panic.c` to trace the leak. Most "Non-present Page" errors are caused by using a pointer after it has been freed.

---
