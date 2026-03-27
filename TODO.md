# TODO

## Debugger (Option B — VM DAP server)

The pico-panda VM already interprets bytecode, making source-level debugging fully
self-contained — no C compiler or GDB needed.

### VM side (`src/`)

- [ ] **Breakpoint table** — add a breakpoint list to `VM`; before executing each instruction check if `task.pc` is in the table and pause if so.
- [ ] **Single-step mode** — per-task flag; VM pauses after each instruction and waits for a DAP `next`/`stepIn`/`continue` command.
- [ ] **DAP server mode** (`mpd vm --dap`)  — run the VM with a DAP server over stdio (JSON messages, same `Content-Length` framing as LSP). Handles: `initialize`, `launch`, `setBreakpoints`, `continue`, `next`, `stepIn`, `stackTrace`, `scopes`, `variables`, `evaluate`.
- [ ] **Stack trace response** — expose task `pc`, `sp`, `frame` as DAP stack frames. Map bytecode addresses back to ASM source lines via a symbol table embedded in the bytecode or sidecar file.
- [ ] **Variable inspection** — expose the evaluation stack slots and R0–R3 registers as DAP variables.

### VS Code extension side

- [ ] **`microPanda.vm` debug type** — DAP client config in the extension that launches `mpd vm --dap <program.ppb>` and connects the VS Code debugger UI to it.
- [ ] **Source mapping** — map bytecode addresses to pico-panda ASM source lines so breakpoints set in `.ppa` files work correctly.
