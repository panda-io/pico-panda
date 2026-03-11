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
- 4 general-purpose registers: R0–R3 (for passing event data)
- Cooperative multitasking: tasks yield via sleep or event wait, no preemption

**Static limits (MCU)**
All pool sizes are compile-time constants — no dynamic allocation at runtime.

## Directory Structure

```plaintext
docs/
  vm.md       VM specification
  asm.md      ASM specification
  signal.md   @signal annotation system
reference/    GDScript reference implementation (Godot)
src/          Micro-Panda implementation (future)
```
