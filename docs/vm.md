# Pico-Panda VM Specification

A stack-based bytecode virtual machine with cooperative multitasking and an event system.
Designed for constrained hardware (MCU) and hosted platforms.

---

## Types

The VM has four value types. All values occupy a 32-bit slot in the eval stack and local variable slots.

| Type | Slot | In `byte[]` | Notes |
| :--- | :---: | :---: | :--- |
| `int` | 4 bytes | — | 32-bit signed integer |
| `fixed` | 4 bytes | — | 16.16 fixed-point (same bits as int) |
| `bool` | 4 bytes | — | 0 = false, 1 = true |
| `byte` | 4 bytes | 1 byte | u8, zero-extended in slots; 1 byte in `byte[]` arrays |

`byte` alone is rarely used as a variable. Its primary purpose is `byte[]` — raw buffers and strings.

---

## Memory Model

### Global Segment

A flat byte array in VM memory, allocated once on startup and zero-filled.
The compiler assigns every global variable a **byte offset** at compile time.

Alignment rules:

- `bool`, `int`, `fixed` scalars: 4 bytes, 4-byte aligned
- `byte` scalar: 1 byte, no alignment requirement (stored as raw byte in memory, widened to int when loaded to stack)
- `byte[N]` inline array: N bytes, 4-byte aligned
- `int[N]`, `fixed[N]`, `bool[N]`: N × 4 bytes, 4-byte aligned
- Struct: fields in declaration order; each field follows the rule above; total size rounded up to multiple of 4

Example layout:

```micro-panda
var score: int       → offset 0    (4 bytes)
var gravity: fixed   → offset 4    (4 bytes)
var alive: bool      → offset 8    (4 bytes)
var buf: byte[256]   → offset 12   (256 bytes)
var pool: int[64]    → offset 268  (256 bytes, next 4-byte boundary)
var view: byte[]     → offset 524  (4 bytes — packed dword: len:u16 | ptr:u16)
```

The assembler emits a `global_size` value in the bytecode header; the VM allocates exactly that many bytes and zero-fills them before executing any instruction.

### Slice Encoding

A slice (`T[]`) is a **packed 32-bit integer** — one stack slot, no hidden struct.

```plaintext
 31          16 15           0
 ┌────────────┬──────────────┐
 │  len (u16) │  ptr  (u16)  │
 └────────────┴──────────────┘
```

Both fields are 16-bit unsigned values. This works because both the global segment
and the code segment are at most 64 KB (16-bit addressable).

- `ptr = dword & 0xFFFF` — byte offset into the global segment
- `len = (dword >> 16) & 0xFFFF` — number of elements

The compiler emits the bit extraction and `MEM_LOAD_*` / `MEM_STORE_*` opcodes
for `s[i]` and `s.len` accesses. Programs never pack or unpack the dword manually.

### Constant Pool

- Read-only byte array appended after instructions in the bytecode image
- Stores string data as: `[length: u16 LE][bytes...]`
- Addresses are byte offsets from the start of the bytecode image

### Per-Task Memory

Each task has independent:

| Region | Size | Description |
| :--- | :--- | :--- |
| Evaluation stack | 8 × 4 bytes | LIFO operand stack |
| Local variables | 8 frames × 16 slots × 4 bytes | Per-frame locals |
| Program counter | u16 | Current instruction offset |
| Stack pointer | u8 | Next free stack slot (0–7) |
| Frame pointer | u8 | Current call frame (0–7) |

---

## Task States

```plaintext
RUNNING  ──► SLEEPING   (HANDLER_SLEEP)
RUNNING  ──► FINISHED   (EXIT_HANDLER or program end)
SLEEPING ──► RUNNING    (sleep timer expires)
```

---

## Execution Model

Each VM `tick()` performs in order:

1. Decrement sleep timers for all sleeping tasks; wake those that reach 0
2. **Process entire event queue** — for each queued event, spawn a handler task for every registered handler (in registration order)
3. Execute all RUNNING tasks until each sleeps or finishes

Event processing happens exclusively at the **start of each tick**, before any task executes. This makes execution fully deterministic: given the same event queue, the same tick always produces the same result.

Tasks are cooperative — they run until they explicitly yield (sleep) or finish. There is no preemption.

> **Note:** This tick-based event model is specific to the Pico-Panda VM. Native Micro-Panda processes signals immediately upon arrival, enabling high-frequency applications (e.g. flight controllers) where latency matters.

---

## Signal IDs

Signals are identified by a 32-bit integer (`event_id`) in bytecode.

### System signals

A small set of signals are pre-assigned fixed IDs. Both the assembler and the host C code use these constants:

| ID | Constant | Description |
| :---: | :--- | :--- |
| 1 | `SIGNAL_SYS_START` | Fired once on system VM startup, before the first tick |
| 2 | `SIGNAL_APP_START` | Fired once on app VM startup, before the first tick |
| 3 | `SIGNAL_TICK` | Fired every tick by the host driver |

The host dispatches them by constant value:

```c
vm_send(vm, SIGNAL_SYS_START, NULL);   // SIGNAL_SYS_START = 1
vm_send(vm, SIGNAL_TICK,      NULL);   // SIGNAL_TICK       = 3
```

### User-defined signals

Any signal name not in the system table is **user-defined**. The assembler collects all user signal names in pass 1 and assigns sequential IDs starting at `0x100`:

```plaintext
first user signal  → 0x100
second user signal → 0x101
...
```

User signals are only sent and received within the bytecode itself (via `EVENT SEND` / `@signal`), so the host never needs to know their IDs.

Signal names are **strings in ASM source only** — no names are stored in bytecode.

---

## Call Convention

`CALL <addr>` (with arg count on top of stack):

1. Pop `arg_count` from stack
2. Pop `arg_count` arguments into `locals[frame][0..n-1]` (arg0 → local 0)
3. Push current PC as return address
4. Increment frame counter
5. Set PC to `addr`

`RET`:

1. Pop return value
2. Pop return address
3. Decrement frame counter
4. Push return value

**Recursion is not allowed.** The fixed 8-frame pool is what keeps memory usage static and predictable on MCU.

---

## Instruction Set

All opcodes are 1 byte. Multi-byte operands are little-endian.

### Data

| Mnemonic | Opcode | Operand | Description |
| :--- | :---: | :--- | :--- |
| `PUSH <val>` | 0x01 | i32 (4 bytes) | Push 32-bit immediate |

> `LOAD_GLOBAL`, `STORE_GLOBAL`, `LOAD_LOCAL`, `STORE_LOCAL`, `ADDR_GLOBAL`, and `MEM_*` are **not included in v1**.
> They will be designed once struct support requirements are clearer — the right encoding depends on whether fields stay uniform 4-byte slots or need variable-size access.

### Stack

| Mnemonic | Opcode | Description |
| :--- | :---: | :--- |
| `DUP` | 0x18 | Duplicate top of stack |
| `SWAP` | 0x19 | Swap top two elements |
| `DROP` | 0x1A | Discard top element |

### Integer Arithmetic

Binary ops pop two values (second-from-top OP top); unary ops pop one.

| Mnemonic | Opcode | Description |
| :--- | :---: | :--- |
| `ADD_INT` | 0x20 | a + b |
| `SUB_INT` | 0x21 | a - b |
| `MUL_INT` | 0x22 | a * b |
| `DIV_INT` | 0x23 | a / b (signed) |
| `MOD_INT` | 0x24 | a % b |
| `NEG_INT` | 0x25 | -a |
| `SHL_INT` | 0x26 | a << b |
| `SHR_INT` | 0x27 | a >> b (logical, zero-fill) |
| `SAR_INT` | 0x28 | a >> b (arithmetic, sign-fill) |
| `AND` | 0x29 | a & b |
| `OR` | 0x2A | a \| b |
| `XOR` | 0x2B | a ^ b |
| `NOT` | 0x2C | ~a |

### Fixed-Point Arithmetic (16.16)

| Mnemonic | Opcode | Description |
| :--- | :---: | :--- |
| `ADD_FIXED` | 0x40 | a + b |
| `SUB_FIXED` | 0x41 | a - b |
| `MUL_FIXED` | 0x42 | (a × b) >> 16 |
| `DIV_FIXED` | 0x43 | (a << 16) / b |
| `NEG_FIXED` | 0x44 | -a |
| `INT_TO_FIXED` | 0x45 | a << 16 |
| `FIXED_TO_INT` | 0x46 | a >> 16 (truncates toward zero) |

### Comparison

Push 1 (true) or 0 (false). Pop two values; second-from-top compared to top.

| Mnemonic | Opcode | | Mnemonic | Opcode |
| :--- | :---: | :--- | :--- | :---: |
| `CMP_EQ_INT` | 0x50 | | `CMP_EQ_FIXED` | 0x56 |
| `CMP_NE_INT` | 0x51 | | `CMP_NE_FIXED` | 0x57 |
| `CMP_LT_INT` | 0x52 | | `CMP_LT_FIXED` | 0x58 |
| `CMP_LE_INT` | 0x53 | | `CMP_LE_FIXED` | 0x59 |
| `CMP_GT_INT` | 0x54 | | `CMP_GT_FIXED` | 0x5A |
| `CMP_GE_INT` | 0x55 | | `CMP_GE_FIXED` | 0x5B |

### Control Flow

| Mnemonic | Opcode | Operand | Description |
| :--- | :---: | :--- | :--- |
| `JMP <label>` | 0x60 | u16 address | Unconditional jump |
| `JMP_IF_TRUE <label>` | 0x61 | u16 address | Jump if pop != 0 |
| `JMP_IF_FALSE <label>` | 0x62 | u16 address | Jump if pop == 0 |
| `CALL <label>` | 0x63 | u16 address | Call (see Call Convention) |
| `RET` | 0x64 | — | Return from call |

### System Calls

`SYSCALL <subcode>` — opcode 0x70 followed by 1-byte subcode.

| Subcode | Name | Stack effect | Description |
| :---: | :--- | :--- | :--- |
| 0x01 | `PRINT_INT` | pop int | Print integer |
| 0x02 | `PRINT_FIXED` | pop fixed | Print fixed-point as decimal |
| 0x03 | `PRINT_STR` | pop addr | Print string from constant pool at addr |

### Signal module (`MODULE_SIGNAL = 0x02`)

`CALL_MODULE 0x02 <subcode>` — or the planned `SIGNAL <subcode>` assembler mnemonic.

| Subcode | Name | Stack effect | Description |
| :---: | :--- | :--- | :--- |
| 0x01 | `CREATE_HANDLER` | pop addr, pop signal_id | Register handler at addr for signal_id (i32) |
| 0x02 | `EXIT_HANDLER` | — | Mark current task as FINISHED |
| 0x03 | `HANDLER_SLEEP` | pop ticks | Suspend task for N ticks |
| 0x04 | `SEND` | pop signal_id | Queue signal (signal_id is i32) |

---

## Bytecode Image Format

```plaintext
[ Header ]         8 bytes
[ Instructions ]   variable length
[ Constant Pool ]  variable length, immediately follows instructions
```

Header layout (8 bytes, all little-endian):

```plaintext
[magic: u16 = 0x5050]  "PP"
[version: u8]
[reserved: u8]
[global_size: u16]     bytes to allocate for the global segment
[instr_size: u16]      byte length of the instruction section
```

Constant pool entries (strings):

```plaintext
[length: u16 LE][bytes: u8 × length]
```

---

## Limits (MCU build)

| Resource | Default limit |
| :--- | :---: |
| Max concurrent tasks | 8 |
| Max call depth per task | 8 |
| Locals per frame | 16 |
| Eval stack depth | 8 |
| Global segment size | 4 KB |
| Event queue depth | 16 |
| Event handlers per signal | 4 |

All limits are compile-time constants. HOSTED builds may use dynamic sizing.
