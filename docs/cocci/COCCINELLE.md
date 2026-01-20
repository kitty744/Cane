# Semantic Patching with Coccinelle (spatch)

To maintain the stability of the CaneOS kernel, we use **Coccinelle** for static analysis. This allows us to find complex logic bugs, such as memory leaks and spinlock deadlocks, that standard compilers like GCC cannot detect.

## 2. Configuration

The project includes a `.cocciconfig` file in the root directory. This file automatically configures:

- **Include Paths:** Points to `include/` so the tool understands our kernel types.
- **Macros:** Uses `scripts/cocci/cocci_macros.h` to handle `asm volatile` and attributes.
- **Performance:** Sets the timeout and number of parallel jobs.

## 3. Available Scripts

Our custom rules are located in `scripts/cocci/`.

| Script                 | Purpose                                                                 |
| :--------------------- | :---------------------------------------------------------------------- |
| `lock_check.cocci`     | Ensures every `spinlock_acquire` has a matching `release`.              |
| `double_free.cocci`    | Detects multiple `free()` calls on the same pointer.                    |
| `use_after_free.cocci` | Flags any access to a pointer after it has been freed.                  |
| `null_check.cocci`     | Verifies that `malloc` and `vmm_alloc` results are checked.             |
| `protect_magic.cocci`  | Ensures `heap_node->magic` isn't tampered with outside the heap driver. |

## 4. How to Run

### Scan a specific file

```bash
spatch --sp-file scripts/cocci/lock_check.cocci mm/heap.c
```

### Scan the entire memory management directory

```bash
spatch --sp-file scripts/cocci/null_check.cocci --dir mm/
```

### Automated Check (via Makefile)

We have integrated these checks into the build system. To run all safety scripts against the entire codebase:

```bash
make coccinelle
```

## 5. Interpreting Results

- **No Output:** This is the goal! It means no bugs were found.
- **Diff Output (- and +):** Coccinelle suggests a fix (e.g., adding a missing `spinlock_release`).
- **Highlighted Lines (\*):** These lines indicate a logic error that needs manual review.

---

_Note: Always run `make coccinelle` before submitting a Pull Request to ensure the kernel heap remains stable._
