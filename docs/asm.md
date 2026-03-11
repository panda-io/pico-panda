# Pico-Panda ASM Specification

Text assembly language that compiles to Pico-Panda bytecode.
One mnemonic per line. Two-pass assembly for forward label references.

The assembler accepts source as either a **file path** or an **in-memory string**.
String input is the primary interface for unit testing — no file I/O required.

---

## Source Format

```asm
; this is a comment

.global score: int = 0      ; global variable declarations (top level)
.global buf: byte[256]

@signal(tick)               ; annotation before fun
fun update:
    .local count            ; local slot alias (inside fun only)
    LOAD_GLOBAL score
    PUSH 1
    ADD_INT
    STORE_GLOBAL score
    EVENT EXIT_HANDLER
```

- **Comments**: `;` to end of line
- **Labels**: identifier followed by `:` on its own (or inline before a mnemonic)
- **Directives**: `.global`, `.local` — assembler-only, emit no bytecode
- **Annotations**: `@signal(name)` before `fun` — generates handler registration
- **Mnemonics**: case-insensitive, one per line
- **Whitespace**: leading/trailing ignored; tokens separated by spaces

---

## Declarations

### `.global name: type [= value]`

Declares a global variable. The assembler assigns it a byte offset in the global segment.
All names in instructions are resolved to their offsets at assemble time.

```asm
.global score: int = 0          ; 4 bytes, offset 0
.global gravity: fixed = 1.0    ; 4 bytes, offset 4
.global alive: bool = 1         ; 4 bytes, offset 8
.global buf: byte[256]          ; 256 bytes, offset 12
.global pool: int[64]           ; 256 bytes, offset 268 (next 4-byte boundary)
```

Supported types:

| Declaration | Size | Notes |
| :--- | :---: | :--- |
| `int` | 4 bytes | 32-bit signed |
| `fixed` | 4 bytes | 16.16 fixed-point |
| `bool` | 4 bytes | 0 or 1 |
| `byte` | 1 byte | raw byte (use `byte[]` in practice) |
| `byte[N]` | N bytes | raw buffer or string storage |
| `int[N]` | N × 4 bytes | int array |
| `fixed[N]` | N × 4 bytes | fixed array |

Initial values in `.global` declarations are zero-filled in the global segment.
String defaults (`= "hello"`) are **not** supported — copy from constant pool explicitly at `@signal(start)`.

### `.local name`

Declares a named alias for the next available local slot within the current `fun`.
No type — slots are untyped 4-byte cells. Names are resolved to slot indices at assemble time.

```asm
fun my_func:
    .local x        ; slot 0
    .local y        ; slot 1
    .local temp     ; slot 2

    PUSH 10
    STORE_LOCAL x   ; → STORE_LOCAL 0
    PUSH 20
    STORE_LOCAL y   ; → STORE_LOCAL 1
    LOAD_LOCAL x
    LOAD_LOCAL y
    ADD_INT
    RET
```

`.local` declarations must appear at the top of `fun`, before any instructions.
Slot numbering starts at 0. Arguments passed by the caller occupy the first N slots automatically.

---

## Signal Annotation

### `@signal(name)`

Placed before a `fun` declaration to register that function as a handler for the named signal.

```asm
@signal(start)
fun init:
    ; runs once at startup
    EVENT EXIT_HANDLER

@signal(tick)
fun update:
    ; runs every tick
    EVENT EXIT_HANDLER

@signal(tick, 3)
fun slow_update:
    ; runs every 3 ticks
    EVENT EXIT_HANDLER
```

The assembler automatically generates `CREATE_HANDLER` calls for each `@signal` function
at the beginning of the bytecode, before any user code executes.

### Signal ID assignment

Signal names are **strings in ASM source only** — no names are stored in bytecode.
The assembler resolves each name to an `i32` event_id using two rules:

**System signals** — fixed IDs that the host C code also uses as constants:

| ID | Name |
| :---: | :--- |
| 1 | `start` |
| 2 | `tick` |

**User-defined signals** — any name not in the system table.
The assembler collects them in pass 1 and assigns sequential IDs starting at `0x100`:

```asm
@signal(explode)    ; first user signal  → event_id = 0x100
@signal(respawn)    ; second user signal → event_id = 0x101
```

User signals are only sent/received within the bytecode (via `EVENT SEND`).
The host never needs to know their IDs.

---

## Operand Types

| Type | Format | Encoding |
| :--- | :--- | :--- |
| Integer literal | `42`, `-7`, `0xFF` | 4-byte LE i32 |
| Fixed literal | `1.5`, `-0.25` | 4-byte LE i32 (converted at assemble time) |
| String literal | `"hello"` | Constant pool reference (4-byte LE offset) |
| Label / fun name | `start`, `update` | 2-byte LE address |
| Global name | `score`, `buf` | 2-byte LE byte offset (resolved from `.global`) |
| Local name | `x`, `temp` | 1-byte slot index (resolved from `.local`) |
| Slot index | `0`–`15` | 1-byte |
| Subcode | `PRINT_INT` | 1-byte |

---

## Pseudo-Opcodes

These are assembler conveniences — they do not correspond to a unique opcode.

### `PUSH_FLOAT <float>`

Syntactic sugar for pushing a floating-point value as 16.16 fixed-point.

```asm
PUSH_FLOAT 1.5      ; equivalent to PUSH 98304   (0x00018000)
PUSH_FLOAT -0.5     ; equivalent to PUSH -32768  (0xFFFF8000)
```

Emits a standard `PUSH` (0x01) with the converted 4-byte fixed value.

### `PUSH_STR <"string">`

Stores the string in the constant pool and pushes its byte address.

```asm
PUSH_STR "hello"    ; push constant pool address of "hello"
SYSCALL PRINT_STR
```

Emits a standard `PUSH` (0x01) with the 4-byte constant pool offset.
Each unique string is stored once.

---

## Instruction Reference

See `vm.md` for full opcode details. Summary of operand syntax:

```asm
; Scalar global (4-byte read/write by name or offset)
LOAD_GLOBAL score       ; name → 2-byte byte offset
STORE_GLOBAL score
LOAD_GLOBAL 0           ; raw offset also accepted

; Pointer to global (for arrays and struct fields)
ADDR_GLOBAL buf         ; push byte address of buf[0]
ADDR_GLOBAL pool

; Memory access through pointer (addr on stack)
MEM_LOAD_INT            ; pop addr → push 4-byte int
MEM_STORE_INT           ; pop int, pop addr → store
MEM_LOAD_BYTE           ; pop addr → push byte (zero-extended)
MEM_STORE_BYTE          ; pop int (low byte), pop addr → store byte

; Locals (by name or slot index)
LOAD_LOCAL x            ; name → slot index
STORE_LOCAL x
LOAD_LOCAL 0            ; raw slot index also accepted
STORE_LOCAL 0

; Immediate
PUSH 42
PUSH_FLOAT 1.5
PUSH_STR "hello"

; Stack
DUP   SWAP   DROP

; Integer arithmetic
ADD_INT  SUB_INT  MUL_INT  DIV_INT  MOD_INT
NEG_INT
SHL_INT  SHR_INT  SAR_INT
AND  OR  XOR  NOT

; Fixed-point arithmetic
ADD_FIXED  SUB_FIXED  MUL_FIXED  DIV_FIXED
NEG_FIXED
INT_TO_FIXED
FIXED_TO_INT

; Comparison (push 1 or 0)
CMP_EQ_INT  CMP_NE_INT  CMP_LT_INT  CMP_LE_INT  CMP_GT_INT  CMP_GE_INT
CMP_EQ_FIXED  CMP_NE_FIXED  CMP_LT_FIXED  CMP_LE_FIXED  CMP_GT_FIXED  CMP_GE_FIXED

; Control flow (labels only, not raw addresses)
JMP label
JMP_IF_TRUE label
JMP_IF_FALSE label
CALL label          ; arg_count must be on top of stack
RET

; System calls
SYSCALL PRINT_INT
SYSCALL PRINT_FIXED
SYSCALL PRINT_STR

; Events
EVENT CREATE_HANDLER    ; stack: [event_id, addr] — event_id is i32
EVENT EXIT_HANDLER
EVENT HANDLER_SLEEP     ; stack: [ticks]
EVENT SEND              ; stack: [event_id]
```

---

## Label and Fun Rules

- `fun name:` declares a function — creates a label and opens a `.local` scope
- Plain `label:` also valid for internal jump targets (no `.local` scope)
- Labels must be unique within a file
- Labels may be forward-referenced (resolved in pass 2)
- A label refers to the byte offset of the next instruction

---

## Call Convention in ASM

Caller pushes arguments left-to-right, then pushes the count, then calls:

```asm
PUSH 10         ; arg 0
PUSH 20         ; arg 1
PUSH 2          ; arg count
CALL add

; add receives: local 0 = 10, local 1 = 20
fun add:
    .local a    ; slot 0 (arg 0)
    .local b    ; slot 1 (arg 1)
    LOAD_LOCAL a
    LOAD_LOCAL b
    ADD_INT
    RET
```

---

## Array Access Patterns

### `byte[]` buffer (raw memory)

```asm
.global buf: byte[256]

; buf[i] = 65
ADDR_GLOBAL buf     ; push base address of buf
LOAD_LOCAL i        ; push index
ADD_INT             ; addr = base + i
PUSH 65
MEM_STORE_BYTE

; x = buf[i]
ADDR_GLOBAL buf
LOAD_LOCAL i
ADD_INT
MEM_LOAD_BYTE       ; push byte (zero-extended to int)
STORE_LOCAL x
```

### `int[]` array

```asm
.global pool: int[64]

; pool[i] = val
ADDR_GLOBAL pool
LOAD_LOCAL i
PUSH 4
MUL_INT             ; byte offset = i * 4
ADD_INT
LOAD_LOCAL val
MEM_STORE_INT

; x = pool[i]
ADDR_GLOBAL pool
LOAD_LOCAL i
PUSH 4
MUL_INT
ADD_INT
MEM_LOAD_INT
STORE_LOCAL x
```

---

## Two-Pass Assembly

Input is split into lines before pass 1. File and string sources are identical after this point.

### Pass 1

- Collect all `.global` declarations → build offset table, compute `global_size`
- Collect all `@signal` names → assign event IDs (system signals get fixed IDs; user signals get sequential IDs from `0x100`)
- Walk instructions; record each `fun`/`label` → current byte offset
- Count `.local` declarations per `fun` (slot numbering)

### Pass 2

- Emit bytecode header (magic, version, `global_size`, `instr_size`)
- Emit handler registrations for all `@signal` functions (before user code)
- For each instruction, emit opcode + operands, resolve names to offsets/slots/addresses
- Append constant pool after all instructions

### Instruction byte sizes

| Instruction | Size |
| :--- | :---: |
| No operand (arithmetic, stack ops) | 1 |
| `SYSCALL` / `EVENT` | 2 (opcode + 1-byte subcode) |
| `JMP*` / `CALL` | 3 (opcode + 2-byte address) |
| `PUSH` / `PUSH_FLOAT` / `PUSH_STR` | 5 (opcode + 4-byte value) |

> `LOAD_LOCAL`, `STORE_LOCAL`, `LOAD_GLOBAL`, `STORE_GLOBAL`, `ADDR_GLOBAL`, and `MEM_*` are **not in v1** — sizes TBD.

---

## Examples

```asm
; Global counter incremented each tick

.global score: int = 0

@signal(start)
fun init:
    EVENT EXIT_HANDLER

@signal(tick)
fun update:
    LOAD_GLOBAL score
    PUSH 1
    ADD_INT
    STORE_GLOBAL score
    EVENT EXIT_HANDLER
```

```asm
; Function call with named locals

.global result: int = 0

@signal(start)
fun init:
    PUSH 3
    PUSH 4
    PUSH 2
    CALL add
    STORE_GLOBAL result
    EVENT EXIT_HANDLER

fun add:
    .local a    ; slot 0
    .local b    ; slot 1
    LOAD_LOCAL a
    LOAD_LOCAL b
    ADD_INT
    RET
```

```asm
; byte[] as string buffer

.global msg: byte[16]

@signal(start)
fun init:
    ; write 'H','i' into msg[0], msg[1]
    ADDR_GLOBAL msg
    PUSH 0
    ADD_INT
    PUSH 72         ; 'H'
    MEM_STORE_BYTE

    ADDR_GLOBAL msg
    PUSH 1
    ADD_INT
    PUSH 105        ; 'i'
    MEM_STORE_BYTE

    EVENT EXIT_HANDLER
```
