# Signal Annotation System

A compile-time annotation system that eliminates boilerplate entry points.
Instead of writing `main()` or an Arduino-style `loop()`, the programmer declares
signal handlers and the compiler generates the registration and main automatically.

---

## Annotations

### `@signal(start)`

Runs once when the program starts. Use for initialization.

```
@signal(start)
fun on_start()
    // init variables, load assets, register state
```

### `@signal(tick)`

Runs every tick. The tick interval is platform-configured (see below).
Use for game logic, state updates, drawing.

```
@signal(tick)
fun on_tick()
    // update and draw
```

### `@signal(tick, <interval>)`

Runs every tick but only executes the handler every `interval` ticks.
Useful for logic that doesn't need to run at full tick rate.

```
@signal(tick, 10)
fun slow_update()
    // runs every 10 ticks
```

### `@signal("<name>")`

Custom named signal. Any handler can send this signal via `EVENT SEND`.
Multiple handlers can register for the same custom signal.

```
@signal("jump")
fun on_jump()
    // respond to jump event

@signal("jump")
fun on_jump_sfx()
    // also responds to jump — both handlers fire
```

---

## Multiple Handlers

Multiple functions may be annotated with the same signal.
They are dispatched in **registration order** (top-to-bottom in source).
Each runs in its own task slot.

```
@signal(tick)
fun player_update()   // runs first
    ...

@signal(tick)
fun enemy_update()    // runs second
    ...

@signal(tick)
fun draw()            // runs last
    ...
```

Order matters when later handlers depend on state written by earlier ones
(e.g. `draw()` must see updated positions). Control order by source order.

---

## Generated Main

Given the annotations above, the compiler generates the entry point automatically:

```asm
; generated — do not write this by hand
    PUSH player_update
    PUSH EVENT_TICK
    EVENT CREATE_HANDLER

    PUSH enemy_update
    PUSH EVENT_TICK
    EVENT CREATE_HANDLER

    PUSH draw
    PUSH EVENT_TICK
    EVENT CREATE_HANDLER

    PUSH on_jump
    PUSH EVENT_JUMP
    EVENT CREATE_HANDLER

    PUSH on_jump_sfx
    PUSH EVENT_JUMP
    EVENT CREATE_HANDLER

    PUSH on_start
    PUSH EVENT_START
    EVENT CREATE_HANDLER

    EVENT EXIT_HANDLER  ; main task done
```

The programmer writes none of this.

---

## Tick Interval

The tick interval is set at the project level, not in source code.
Platform implementations:

| Platform | Implementation |
| :--- | :--- |
| MCU | RTOS timer task at configured interval |
| Desktop (SDL2) | SDL timer or game loop at configured interval |
| Godot | `_process()` or `_physics_process()` |

Typical values: `16ms` (≈60fps), `33ms` (≈30fps), `1ms` (high-frequency control).

---

## Dispatch Model

### Pico-Panda VM

Events are queued and processed at the **start of each tick** before any task executes.
A signal sent during tick N is received and dispatched at the start of tick N+1.
Fully deterministic — no mid-tick interruption.

### Native Micro-Panda

Signals are processed **immediately** upon arrival — no queue, no tick boundary.
Enables high-frequency applications (flight controllers, sensor loops) where
tick-based latency is unacceptable.

The `@signal` annotation syntax is identical in both. Only the dispatch
mechanism differs, handled entirely by the platform layer.

---

## `wait` / `HANDLER_SLEEP`

Sleeping a task to implement delays is **discouraged**. It occupies a task slot
in the fixed pool while doing nothing.

Prefer periodic logic in a `@signal(tick)` handler with a counter in global state:

```
; discouraged
loop:
    // do work
    PUSH 60
    EVENT HANDLER_SLEEP   // blocks a task slot for 60 ticks
    JMP loop

; preferred
@signal(tick)
fun on_tick()
    // do work every tick — no sleeping, no wasted slot
    EVENT EXIT_HANDLER
```

`HANDLER_SLEEP` remains available for one-shot delays where spawning a
dedicated timer task is genuinely the clearest solution.
