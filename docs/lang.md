# Pico Panda Language Specification

Pico Panda is a small, statically-typed language that compiles to Pico Panda bytecode
and runs on the Pico Panda VM.

It is designed to be **ultra-light** — smaller in scope than Micro Panda itself.
No heap, no OOP, no generics. Every program is a set of signal handlers operating
on global state.

---

## Design Principles

- **No classes.** Behavior lives in standalone functions, not objects.
- **No methods.** `data` types hold fields only. Functions receive data by implicit reference.
- **No copy.** `data` values cannot be assigned to each other. All data is worked on in place.
- **No nested data.** A `data` field can be a primitive or a slice, never another `data` inline. This keeps layouts flat and eliminates implicit copies.
- **No explicit references.** `data` is always global; passing it automatically passes its address. No `&` syntax needed.
- **Signals, not main.** Entry points are declared with `@signal`. The compiler generates all handler registration automatically.
- **Static memory.** All limits (tasks, handlers, stack depth) are compile-time constants. No dynamic allocation in v1.

---

## Primitives

| Type     | Size    | Description                                              |
| :------- | :-----: | :------------------------------------------------------- |
| `int`    | 4 bytes | 32-bit signed integer                                    |
| `fixed`  | 4 bytes | 16.16 fixed-point                                        |
| `bool`   | 4 bytes | `true` or `false`                                        |
| `byte`   | 1 byte  | unsigned 8-bit; widened to int on stack                  |
| `string` | 4 bytes | read-only string constant; packed address+length (1 word)|

`fixed` literals use decimal notation: `1.5`, `-0.25`, `0.0`.
Integer literals: `42`, `-7`, `0xFF`.
Boolean literals: `true`, `false`.
String literals: `"hello"` — stored in the constant pool; passed by value like a slice.

No implicit conversions between types. Use explicit casts: `int(x)`, `fixed(x)`.

---

## Variables

```pico-panda
var score: int = 0          // variable
const MAX_HP: int = 100     // compile-time constant
```

Type inference with `:=`:

```pico-panda
var x := 42         // inferred: int
var g := 9.8        // inferred: fixed
```

---

## Enums

Simple integer enums. Values start at `0` and increment by 1. No data variants.

```pico-panda
enum Direction
    UP
    DOWN
    LEFT
    RIGHT

enum State
    IDLE    = 0
    RUNNING = 1
    DEAD    = 2
```

Use as typed integer constants:

```pico-panda
var dir: Direction = Direction.UP
var s: State = State.IDLE
```

Explicit values are optional. All comparisons use integer ops — `dir == Direction.LEFT`.
Enums work as `match` discriminants (see [Control Flow](#control-flow)).

---

## Arrays

### Fixed arrays

Declared with a compile-time size. Stored in the global segment.
No runtime bounds checking.

```pico-panda
var buf:  byte[256]     // 256 bytes
var nums: int[64]       // 64 × 4 bytes
var pts:  fixed[32]     // 32 × 4 bytes
var flags: bool[8]      // 8 × 4 bytes
```

Element access:

```pico-panda
buf[0] = 72
var \1: byte = buf[i]
nums[i] = nums[i] + 1
```

### Slices

A slice (`T[]`) is a view into a fixed array — an (address, length) pair packed into a
single 32-bit integer. It occupies **one stack slot** and passes by value like any `int`.

**Dword encoding:**

```pico-panda
 31          16 15           0
 ┌────────────┬──────────────┐
 │  len (u16) │  ptr  (u16)  │
 └────────────┴──────────────┘
```

Both fields are 16-bit unsigned values. This works because the global segment and code
segment are each at most 64 KB (16-bit addressable). There is no hidden struct — a slice
variable is just an `int`.

**Creating a slice:**

```pico-panda
var buf: byte[256]
var \1: byte[] = {&buf, 64}      // ptr = address of buf, len = 64
```

**`.len`** — extract the length (high 16 bits):

```pico-panda
var \1: int = s.len              // n = 64
```

**Indexing** — extract ptr (low 16 bits), add element offset, load/store:

```pico-panda
var \1: byte  = s[i]             // byte[]:  ptr + i
var \1: int   = ns[i]            // int[]:   ptr + i*4
var \1: fixed = fs[i]            // fixed[]: ptr + i*4
s[i] = 42
```

The compiler generates the bit extraction and memory opcodes. You never pack or unpack
the dword manually.

**Slices as function parameters:**

Because a slice is just an `int`, it passes and returns by value with no overhead:

```pico-panda
fun sum(data: int[], n: int) int
    var total: int = 0
    var i: int = 0
    while i < n
        total = total + data[i]
        i = i + 1
    return total
```

Or use `.len` directly when the length is encoded in the slice:

```pico-panda
fun sum(data: int[]) int
    var total: int = 0
    var i: int = 0
    while i < data.len
        total = total + data[i]
        i = i + 1
    return total
```

**No `data[]` in v1.** Slices of primitive types only: `byte[]`, `int[]`, `fixed[]`, `bool[]`.

---

## Data

`data` declares a named flat record. Fields must be primitives or slices (`T[]`).
No methods. No nested `data` inline.

```pico-panda
data Vec2
    x: fixed
    y: fixed

data Sprite
    hp:   int
    name: byte[]    // slice — OK (a dword field)
    // body: Vec2   // ERROR: nested data not allowed
```

### Declaring data variables

`data` variables must be global. They are zero-initialized by default. Set fields explicitly:

```pico-panda
var player: Sprite
player.hp   = 100
player.name = {&name_buf, 6}
```

### No copy

You cannot assign one `data` value to another:

```pico-panda
var a: Sprite
var b: Sprite
b = a               // ERROR: cannot copy data
b.hp = a.hp         // OK — copy a single primitive field explicitly
```

### Pass by implicit reference

`data` types are always passed by implicit reference — no `&` syntax needed.
The compiler automatically passes the address of the global variable:

```pico-panda
fun update(s: Sprite)   // s is implicitly a reference to a global Sprite
    s.hp = s.hp - 1

var g_player: Sprite
update(g_player)        // compiler passes &g_player automatically
```

`data` types cannot be returned by value or declared on the stack:

```pico-panda
fun find(): Sprite      // ERROR: data cannot be returned by value
fun f()
    var s: Sprite       // ERROR: data cannot be declared on the stack — use a global
```

---

## Functions

Standalone functions only. No methods on `data`.

```pico-panda
fun move(s: Sprite, dx: fixed, dy: fixed)
    s.pos.x = s.pos.x + dx
    s.pos.y = s.pos.y + dy

fun clamp(v: int, lo: int, hi: int) int
    if v < lo
        return lo
    if v > hi
        return hi
    return v
```

Recursion is **not allowed** — the VM has a fixed call-frame pool (8 frames).

---

## Signals

Signal handlers are top-level functions annotated with `@signal`.
The compiler generates all handler registration — you write no entry point.

```pico-panda
@signal(start)
fun on_start()
    // runs once at startup

@signal(tick)
fun on_tick()
    // runs every tick

@signal(tick, 10)
fun on_slow_tick()
    // runs every 10 ticks

@signal("jump")
fun on_jump()
    // runs when the "jump" user signal is sent
```

Only top-level functions may be signal handlers.
Signal handlers take no parameters and return nothing.

See `signal.md` for the full signal system.

---

## Control Flow

### if / else

```pico-panda
if x > 0
    do_something()

if x > 0
    do_something()
else
    do_other()
```

### while

```pico-panda
while active
    update()
```

### for — range

Half-open range `lo..hi` (lo inclusive, hi exclusive):

```pico-panda
for i in 0..10
    process(i)

for i in 0..buf.size()
    buf[i] = 0
```

### for — collection

Iterate over a fixed array or slice. Optionally capture the index:

```pico-panda
for item in nums
    total = total + item
```

`break` and `continue` work inside all loop forms.

### match

Exhaustive match on an integer, `bool`, or `enum` value.
Each arm is a single expression or an indented block.
`_` is the wildcard (default) arm — required if not all values are covered.

```pico-panda
match dir
    Direction.UP    → move_up()
    Direction.DOWN  → move_down()
    Direction.LEFT  → move_left()
    Direction.RIGHT → move_right()

match lives
    0 → game_over()
    _ → continue_game()

match state
    State.IDLE →
        score = 0
        lives = 3
    State.RUNNING → update()
    _ → {}
```

Only one arm fires. Arms are checked top-to-bottom; the first match wins.

---

## Operators

### Arithmetic

| Operator    | Types      | Description    |
| :---------: | :--------- | :------------- |
| `+`         | int, fixed | Addition       |
| `-`         | int, fixed | Subtraction    |
| `*`         | int, fixed | Multiplication |
| `/`         | int, fixed | Division       |
| `%`         | int        | Modulo         |
| `-` (unary) | int, fixed | Negation       |

### Bitwise

| Operator | Types | Description           |
| :------: | :---- | :-------------------- |
| `&`      | int   | Bitwise AND           |
| `\|`     | int   | Bitwise OR            |
| `^`      | int   | Bitwise XOR           |
| `~`      | int   | Bitwise NOT           |
| `<<`     | int   | Shift left            |
| `>>`     | int   | Shift right (logical) |

### Comparison

`==`, `!=`, `<`, `<=`, `>`, `>=` — works on `int` and `fixed`. Returns `bool`.

### Logical

`&&`, `||`, `!` — works on `bool`.

### Assignment

`=`, `+=`, `-=`, `*=`, `/=`, `%=`

---

## Explicit Casts

No implicit conversions. Use explicit cast syntax:

```pico-panda
var \1: fixed = fixed(score)     // int → fixed (shift left 16)
var \1: int   = int(gravity)     // fixed → int (shift right 16, truncates)
var \1: byte  = byte(flags)      // int → byte (low 8 bits)
```

---

## What Is Intentionally Excluded

| Feature | Reason |
| :--- | :--- |
| `class` with methods | Signals + standalone functions are sufficient; methods complicate signal dispatch |
| Tagged enums (Rust-style) | No heap; `data` + discriminant field achieves the same goal with flat layout |
| Nested `data` | Prevents implicit copies; keeps layouts flat |
| Explicit `&` references | `data` is always global; implicit pass-by-reference is simpler and safer |
| `data[]` arrays | No heap; field-offset math not in VM v1 |
| Generics | Ultra-light scope; not needed for VM programs |
| Heap allocation | v1 is static memory only |
| Strings as first-class values | Use `byte[]` slices; string data lives in constant pool |
| Exceptions / error handling | Use return codes or signal-based state |
| Recursion | Fixed call-frame pool guarantees static stack usage |

---

## Relation to Micro Panda

Pico Panda shares syntax conventions with Micro Panda (`var`/`const`, `fun`,
`@signal`, `T[]` slices, explicit casts) but is a separate, smaller language
that compiles to bytecode rather than C. References (`&T`) exist in Micro Panda
but are absent from Pico Panda — `data` types are always global and passed by
implicit reference.

Micro Panda is used to **implement** the Pico Panda VM (`vm.mpd`, `task.mpd`).
Pico Panda programs run **on** that VM.

| | Micro Panda | Pico Panda |
| :--- | :--- | :--- |
| Compiles to | C | Bytecode |
| Runs on | MCU / desktop natively | Pico Panda VM |
| Classes | Yes | No (`data` only) |
| Generics | Yes | No |
| References (`&T`) | Yes | No (implicit for `data`) |
| Slice encoding | fat pointer (ptr + size_t) | packed dword (len:u16 \| ptr:u16) |
| Heap | Allocator pattern | No (v1) |
| Signals | Immediate dispatch | Tick-queued dispatch |
| Intended for | Firmware, tools, games | VM scripts, MCU game logic |
