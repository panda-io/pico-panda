# Pico-Panda ASM Specification

Text assembly language that compiles to Pico-Panda bytecode.
One mnemonic per line. Two-pass assembly for forward label references.

The assembler accepts source as either a **file path** or an **in-memory string**.
String input is the primary interface for unit testing — no file I/O required.

---

## Source Format

```asm
; this is a comment

.data
.struct Point
.field x
.field y
.end
.global score: i32
.global pos:   Point
.global buf:   i32[64]

.code
PUSH 42
STORE_GLOBAL score
```

- **Comments**: `;` to end of line
- **Labels**: identifier followed by `:` on its own line (or on the same line as a mnemonic)
- **Sections**: `.data` and `.code` switch the assembler between data-declaration and code-emission modes
- **Directives**: `.global`, `.struct`, `.field`, `.end` — assembler-only, emit no bytecode
- **Annotations**: `@signal(name)` before `fun` — generates handler registration *(planned)*
- **Mnemonics**: case-insensitive, one per line
- **Whitespace**: leading/trailing ignored; tokens separated by spaces

---

## Sections

### `.data`

Switches to data-declaration mode. All `.global`, `.struct`, `.field`, and `.end` directives
must appear inside a `.data` block. No bytecode is emitted in this mode.

### `.code`

Switches back to code-emission mode. Instructions must appear inside a `.code` block (or
before any `.data` block — the assembler starts in code mode).

```asm
.data
.global x: i32
.global y: fixed

.code
PUSH 99
STORE_GLOBAL x
```

---

## Declarations

### `.global name: type`

Allocates a named variable in the data segment. The assembler assigns it a byte offset and
makes the name available to `LOAD_GLOBAL`, `STORE_GLOBAL`, `PUSH_ADDR`, `PUSH_SLICE`, and
`PUSH_FIELD_OFFSET` instructions.

```asm
.data
.global score:   i32         ; 4 bytes, offset 0
.global gravity: fixed       ; 4 bytes, offset 4
.global alive:   bool        ; 4 bytes, offset 8
.global pos:     Point       ; size of struct Point, offset 12
.global buf:     i32[64]     ; 64 × 4 = 256 bytes
.global raw:     u8[256]     ; 256 × 4 = 1024 bytes
```

Supported types:

| Declaration | Size | Notes |
| :--- | :---: | :--- |
| `i32` | 4 bytes | 32-bit signed integer |
| `u32` | 4 bytes | 32-bit unsigned integer |
| `u8` | 4 bytes | byte value (word-aligned slot) |
| `fixed` | 4 bytes | 16.16 fixed-point |
| `bool` | 4 bytes | 0 or 1 |
| `StructName` | struct size | must be declared before the `.global` |
| `T[N]` | N × 4 bytes | array; also registers a packed slice for `PUSH_SLICE` |

All scalar types occupy 4 bytes. There is no 1-byte slot type.

Initial values in `.global` declarations are zero-initialised by the host; no explicit
initialiser syntax is supported in the assembler — write initial values in startup code.

### `.struct Name` / `.field fname` / `.end`

Defines a struct type. Each `.field` allocates a 4-byte slot. `.end` records the total size
so that `.global` and `PUSH_FIELD_OFFSET` can use the struct by name.

```asm
.data
.struct Point
.field x        ; offset 0  (4 bytes)
.field y        ; offset 4  (4 bytes)
.end            ; Point size = 8

.global p: Point    ; 8 bytes
```

Structs must be declared before any `.global` that uses them.

### `.local name` *(planned)*

Declares a named alias for the next available local slot within the current `fun`.
No type — slots are untyped 4-byte cells. Names are resolved to slot indices at assemble time.

---

## Signal Annotation *(planned)*

### `@signal(name)`

Placed before a `fun` declaration to register that function as a handler for the named signal.

```asm
@signal(start)
fun init:
    EVENT EXIT_HANDLER

@signal(tick)
fun update:
    EVENT EXIT_HANDLER
```

---

## Operand Types

| Type | Format | Encoding |
| :--- | :--- | :--- |
| Integer literal | `42`, `-7`, `0xFF` | 4-byte LE i32 |
| Fixed literal | `1.5`, `-0.25` | 4-byte LE i32 (converted at assemble time) |
| String literal | `"hello"` | Constant pool reference (4-byte LE offset) |
| Label / fun name | `start`, `update` | 2-byte LE address |
| Global name | `score`, `buf` | 2-byte LE data-segment byte offset |
| Data address | `PUSH_ADDR name` | i32 data-segment byte offset (emitted as `PUSH`) |
| Packed slice | `PUSH_SLICE name` | i32 packed as `(addr << 16) count` (emitted as `PUSH`) |
| Field offset | `PUSH_FIELD_OFFSET Type f` | i32 byte offset within struct (emitted as `PUSH`) |
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

### `PUSH_ADDR <name>`

Pushes the data-segment byte offset of a named global. Use this to pass a pointer to
`LOAD_GLOBAL_IND` / `STORE_GLOBAL_IND` or to do pointer arithmetic.

```asm
PUSH_ADDR score     ; push i32 data address of 'score'
```

Emits `PUSH` (0x01) + 4-byte offset.

### `PUSH_SLICE <name>`

Pushes a packed representation of an array global: `(addr << 16) | count`.

```asm
PUSH_SLICE buf      ; push packed slice: (offset_of_buf << 16) | element_count
```

Emits `PUSH` (0x01) + 4-byte packed value. Only valid for globals declared with `T[N]`.

### `PUSH_FIELD_OFFSET <Type> <field>`

Pushes the byte offset of a named field within a struct type. Add to a base address to get
the field's address.

```asm
PUSH_FIELD_OFFSET Point y   ; push 4 (byte offset of 'y' in Point)
```

Emits `PUSH` (0x01) + 4-byte offset.

### `PUSH_STR <"string">` *(planned)*

Stores the string in the constant pool and pushes its byte address.

---

## Instruction Reference

```asm
; Scalar global access by name (direct — 2-byte offset operand)
LOAD_GLOBAL  score      ; push 4-byte value of global 'score'
STORE_GLOBAL score      ; pop value, store into global 'score'

; Indirect global access (address on stack — no operand)
LOAD_GLOBAL_IND         ; pop addr → push 4-byte value at addr
STORE_GLOBAL_IND        ; pop value, pop addr → store value at addr

; Data-segment address helpers
PUSH_ADDR  buf                   ; push i32 byte address of 'buf'
PUSH_SLICE buf                   ; push packed (addr<<16 | count)
PUSH_FIELD_OFFSET Point y        ; push byte offset of field 'y' in Point

; Immediate
PUSH 42
PUSH 0xFF
PUSH -7
PUSH_FLOAT 1.5

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

; Control flow
JMP label
JMP_IF_TRUE  label
JMP_IF_FALSE label
CALL  label
RET

; Module calls
CALL_MODULE module_id sub_id    ; e.g. CALL_MODULE 0x01 0x01

; Events (planned)
EVENT CREATE_HANDLER
EVENT EXIT_HANDLER
EVENT HANDLER_SLEEP
EVENT SEND
```

---

## Label and Fun Rules

- `fun name:` declares a function — creates a label and opens a `.local` scope *(planned)*
- Plain `label:` is valid for internal jump targets
- Labels must be unique within a file
- Labels may be forward-referenced (resolved in pass 2)
- A label refers to the byte offset of the next instruction

---

## Call Convention in ASM *(planned)*

Caller pushes arguments left-to-right, then pushes the count, then calls:

```asm
PUSH 10         ; arg 0
PUSH 20         ; arg 1
PUSH 2          ; arg count
CALL add

fun add:
    .local a    ; slot 0 (arg 0)
    .local b    ; slot 1 (arg 1)
    LOAD_LOCAL a
    LOAD_LOCAL b
    ADD_INT
    RET
```

---

## Data Access Patterns

### Scalar global (direct)

```asm
.data
.global score: i32
.code

; store
PUSH 42
STORE_GLOBAL score

; load
LOAD_GLOBAL score
```

### Scalar global (indirect via `PUSH_ADDR`)

```asm
.data
.global score: i32
.code

; store via pointer
PUSH 42
PUSH_ADDR score
STORE_GLOBAL_IND

; load via pointer
PUSH_ADDR score
LOAD_GLOBAL_IND
```

### Array element access

Each element is 4 bytes. Compute `base + index * 4` then use `LOAD_GLOBAL_IND` / `STORE_GLOBAL_IND`.

```asm
.data
.global arr: i32[4]
.code

; arr[2] = 33
PUSH 33
PUSH_ADDR arr
PUSH 2
PUSH 4
MUL_INT
ADD_INT
STORE_GLOBAL_IND

; x = arr[2]
PUSH_ADDR arr
PUSH 2
PUSH 4
MUL_INT
ADD_INT
LOAD_GLOBAL_IND
```

### Struct field access

Use `PUSH_ADDR` for the base address, `PUSH_FIELD_OFFSET Type field` for the offset,
`ADD_INT` to get the field address, then `LOAD_GLOBAL_IND` / `STORE_GLOBAL_IND`.

```asm
.data
.struct Point
.field x
.field y
.end
.global p: Point
.code

; p.y = 55
PUSH 55
PUSH_ADDR p
PUSH_FIELD_OFFSET Point y
ADD_INT
STORE_GLOBAL_IND

; v = p.y
PUSH_ADDR p
PUSH_FIELD_OFFSET Point y
ADD_INT
LOAD_GLOBAL_IND
```

---

## Two-Pass Assembly

Input is split into lines before pass 1. File and string sources are identical after this point.

### Pass 1

- Collect all `.data` / `.struct` / `.field` / `.end` / `.global` declarations:
  - Build the struct-sizes table and per-field byte-offset table
  - Assign each global a byte offset in the data segment; record its name
  - Record array globals in the packed-slice table
- Walk instructions; record each `fun` / `label` → current byte offset (PC tracking)

### Pass 2

- Reset PC and mode to `.code`
- For each instruction, emit opcode + operands, resolving names to offsets / addresses
- Directives (`.data`, `.code`, `.struct`, …) are processed for mode-switching only; data-segment
  offsets were already fixed in pass 1

### Instruction byte sizes

| Instruction | Size |
| :--- | :---: |
| No operand (arithmetic, stack ops) | 1 |
| `LOAD_GLOBAL_IND` / `STORE_GLOBAL_IND` | 1 |
| `CALL_MODULE` | 3 (opcode + 2 × 1-byte subcodes) |
| `JMP*` / `CALL` | 3 (opcode + 2-byte address) |
| `LOAD_GLOBAL` / `STORE_GLOBAL` | 3 (opcode + 2-byte data offset) |
| `PUSH` / `PUSH_FLOAT` | 5 (opcode + 4-byte value) |
| `PUSH_ADDR` / `PUSH_SLICE` / `PUSH_FIELD_OFFSET` | 5 (emitted as `PUSH` + 4-byte value) |

---

## Examples

```asm
; Scalar global: store and load

.data
.global score: i32

.code
PUSH 99
STORE_GLOBAL score

LOAD_GLOBAL score
CALL_MODULE 0x01 0x01
CALL_MODULE 0x02 0x02
```

```asm
; Struct field access

.data
.struct Point
.field x
.field y
.end
.global pos: Point

.code
; pos.y = 55
PUSH 55
PUSH_ADDR pos
PUSH_FIELD_OFFSET Point y
ADD_INT
STORE_GLOBAL_IND

; load pos.y
PUSH_ADDR pos
PUSH_FIELD_OFFSET Point y
ADD_INT
LOAD_GLOBAL_IND
CALL_MODULE 0x01 0x01
CALL_MODULE 0x02 0x02
```

```asm
; int[] array: write arr[2] = 33, read it back

.data
.global arr: i32[4]

.code
PUSH 33
PUSH_ADDR arr
PUSH 2
PUSH 4
MUL_INT
ADD_INT
STORE_GLOBAL_IND

PUSH_ADDR arr
PUSH 2
PUSH 4
MUL_INT
ADD_INT
LOAD_GLOBAL_IND
CALL_MODULE 0x01 0x01
CALL_MODULE 0x02 0x02
```

```asm
; Labels and conditional jump

.code
PUSH 0
JMP_IF_FALSE skip
PUSH 1
CALL_MODULE 0x01 0x01
CALL_MODULE 0x02 0x02
skip:
PUSH 99
CALL_MODULE 0x01 0x01
CALL_MODULE 0x02 0x02
```

```asm
; Backward label (count 0..9, output 10)

.code
PUSH 0
loop:
PUSH 1
ADD_INT
DUP
PUSH 10
CMP_LT_INT
JMP_IF_TRUE loop
CALL_MODULE 0x01 0x01
CALL_MODULE 0x02 0x02
```
