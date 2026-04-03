# Pico-Panda

A lightweight stack-based virtual machine designed to run tiny programs on both MCU (microcontrollers) and hosted platforms. Inspired by Pico-8 and MicroPython — load and run small programs dynamically on constrained hardware.

## Goals

- **MCU runtime**: run simple programs on microcontrollers (RP2040, ESP32, etc.) with no OS, no heap GC
- **Tiny game engine**: pixel display, input, fixed-point math — like Pico-8
- **Dynamic loading**: programs stored in flash or received over serial, loaded and run at runtime
- **Scratch bridge**: visual code blocks (Scratch) → Pico-Panda ASM → bytecode

## Roadmap

1. **VM spec + ASM spec** — define the bytecode format and assembly language *(current)*
2. **VM implementation** — bytecode interpreter in Micro-Panda (MCU + HOSTED builds)
3. **Assembler implementation** — text ASM → bytecode in Micro-Panda
4. **Pico-Panda language** — subset of Micro-Panda syntax, compiles to VM bytecode
5. **Scratch bridge** — visual blocks → Pico-Panda ASM

## Platform Targets

| Platform | Allocator | Notes |
| :--- | :--- | :--- |
| MCU | Static (arena) | Fixed pool sizes, no heap, no OS |
| HOSTED | HeapAllocator | Desktop / dev / testing |
| Reference | GDScript | Godot; see `reference/` — for design exploration only |

## Key Design Decisions

### Types (VM level)

- `int` — 32-bit signed integer
- `fixed` — 16.16 fixed-point (same as Micro-Panda `fixed`)
- `bool` — 0 or 1, stored as int

No native float type. Float literals in ASM (`1.5`) are pseudo-syntax — converted to `fixed` by the assembler at compile time.

### String / byte array naming

- VM level: no string type — strings are byte arrays accessed by address
- ASM level: `"..."` string literals → stored in the constant pool, pushed as address
- Language level (future): `str` — read-only, length-prefixed, lives in const ROM
- Implementation type: `byte[]` (preferred over `u8[]` — more readable, matches the language domain)

### Execution model

- Stack machine: 8-slot evaluation stack
- 4 general-purpose registers: R0–R3 (for passing signal data)
- Cooperative multitasking: tasks yield via sleep or signal wait, no preemption

**Static limits (MCU)**
All pool sizes are compile-time constants — no dynamic allocation at runtime.

## Directory Structure

```plaintext
docs/
  vm.md       VM specification
  asm.md      ASM specification
  signal.md   @signal annotation system
reference/    GDScript reference implementation (Godot)
src/          Micro-Panda VM implementation
```

---

## Implementation Status

Roadmap step 2 (VM implementation) is **complete** — 15/15 tests passing.

### Source files (`src/`)

| File | Contents |
| --- | --- |
| `config.mpd` | Compile-time constants — array sizes, memory limits |
| `opcode.mpd` | Opcode + syscall + signal subcode constants |
| `signal.mpd` | `Signal` class + system signal IDs (`SIGNAL_SYS_START=1`, `SIGNAL_APP_START=2`) |
| `task.mpd` | `Task` class (pc, sp, frame, state, stack[8]), `TaskState` enum |
| `handler.mpd` | `Handler` class (signal_id, addr, task slot=-1 means idle) |
| `vm.mpd` | `class VM` + module singleton `var vm: VM` + public API functions |
| `vm_test.mpd` | 15 `@test` functions covering all opcodes |
| `main.mpd` | Stub `main()` for non-test builds |

### Run tests

```shell
mpd test vm_test.mpd    # from pico-panda/
```

### Config constants

```micro-panda
VM_MAX_FRAMES    = 8     // per-task call stack depth
VM_MAX_SIGNALS    = 32    // signal ring buffer size
SYS_MAX_TASKS    = 1     // system context (boot, I/O, scheduler)
SYS_MAX_HANDLERS = 4
APP_MAX_TASKS    = 8     // application context
APP_MAX_HANDLERS = 32
```

---

## VM Architecture

### Single `class VM`

One module-level instance `var vm: VM` with two independent contexts sharing one singal bus:

- **System context**: `_sys_code: u8[]`, `_sys_tasks: Task[SYS_MAX_TASKS]`, `_sys_handlers: Handler[SYS_MAX_HANDLERS]`, `_sys_h_count: i32`
- **App context**: `_app_code: u8[]`, `_app_tasks: Task[APP_MAX_TASKS]`, `_app_handlers: Handler[APP_MAX_HANDLERS]`, `_app_h_count: i32`
- **Signal bus**: `_signals: Signal[VM_MAX_SIGNALS]` ring buffer (`_sig_head`, `_sig_tail`, `_sig_size`)
- **Test output**: `output: i32` — written by SYSCALL_PRINT_INT / SYSCALL_PRINT_FIXED

### Tick cycle (each frame)

1. Wake sleeping tasks (decrement sleep counters)
2. Drain signal queue → dispatch to sys and app handler tables (resume or spawn handler task)
3. Run all RUNNING tasks in both contexts

### Execute loop

`_run_execute(task: &Task, code: u8[], handlers: Handler[], h_count: i32) i32` is the single
shared opcode dispatch. Returns updated `h_count`. `_sys_execute(t)` and `_app_execute(t)` are
slim wrappers that build a handler slice from the fixed array:

```micro-panda
var hs: Handler[] = {&_sys_handlers[0], SYS_MAX_HANDLERS}
_sys_h_count = _run_execute(task, _sys_code, hs, _sys_h_count)
```

### Public API

```micro-panda
vm_reset()
vm_load_sys(code: u8[]) / vm_load_app(code: u8[])
vm_boot_sys() / vm_boot_app()   // spawn task at addr 0
vm_send(id: i32)                // enqueue signal
vm_tick()                       // one full frame
// read vm.output after tick for PRINT_INT result
```

---

## Bytecode Reference

| Opcode | Mnemonic | Operands | Notes |
| --- | --- | --- | --- |
| `0x01` | PUSH | i32 (4-byte LE) | |
| `0x18` | DUP | — | |
| `0x19` | SWAP | — | |
| `0x1A` | DROP | — | |
| `0x20–0x28` | ADD/SUB/MUL/DIV/MOD/NEG/SHL/SHR/SAR INT | — | |
| `0x29–0x2C` | AND/OR/XOR/NOT | — | |
| `0x40–0x46` | Fixed-point 16.16: ADD/SUB/MUL/DIV/NEG, INT_TO_FIXED, FIXED_TO_INT | — | |
| `0x50–0x5B` | CMP_EQ/NE/LT/LE/GT/GE (INT + FIXED) | — | pushes 1 or 0 |
| `0x60` | JMP | u16 addr | |
| `0x61` | JMP_IF_TRUE | u16 addr | |
| `0x62` | JMP_IF_FALSE | u16 addr | |
| `0x63` | CALL | u16 addr | push argc first; args dropped inside loop |
| `0x64` | RET | — | |
| `0x70` + sub | SYSCALL | `0x01` PRINT_INT, `0x02` PRINT_FIXED, `0x03` PRINT_STR | |
| `0x71` + sub | SIGNAL | `0x01` CREATE_HANDLER, `0x02` EXIT_HANDLER, `0x03` HANDLER_SLEEP, `0x04` SEND | |

---

## Key Implementation Notes

- **Named constants as array sizes**: `Task[SYS_MAX_TASKS]` required compiler fixes in
  `parser_types.dart`, `type_array.dart` (`dimNames` list), `generator_type.dart` (`_constInts` map).
- **Handler slice trick**: fixed arrays can't be passed to functions in Micro Panda — build a
  `Handler[]` slice with `{&arr[0], MAX}` and pass that instead.
- **`h_count` by value + return**: `&i32` can't be dereferenced in Micro Panda, so `_run_execute`
  takes `h_count` by value and returns the updated count.
- **C struct ordering**: the generator topologically sorts struct definitions so embedded
  (non-pointer) fields are defined before the containing struct.
- **Compiler**: `../micro-panda-dart/` — rebuild with `./install.sh`.
- **Test helper naming**: `_boot(code)` in `vm_test.mpd` (not `_run`) — avoids conflict with
  `_app_run` / `_sys_run` methods on the VM class.
