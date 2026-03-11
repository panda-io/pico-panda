# Panda VM Instruction Set

This instruction set is designed for a stack-based VM with tasks and syscalls. Opcodes are one byte (0–255), and parameters follow as needed. Opcodes are grouped by function, leaving space for future expansion.


### Data Operations

| Assembly | Description | Example | Opcode |
| :--- | :--- | :--- | :--- |
| PUSH <val> | Push word onto stack | PUSH 42 | 0x01 |
| LOAD_GLOBAL <idx> | Push global/shared variable | LOAD_GLOBAL 5 | 0x02 |
| STORE_GLOBAL <idx> | Store to global/shared variable | STORE_GLOBAL 5 | 0x03 |
| LOAD_LOCAL <idx> | Push task-local variable | LOAD_LOCAL 2 | 0x04 |
| STORE_LOCAL <idx> | Store to task-local variable | STORE_LOCAL 2 | 0x05 |

### Stack Operations

| Assembly | Description | Example | Opcode |
| :--- | :--- | :--- | :--- |
| PUSH_R0 | Push register R0 onto stack | PUSH_R0 | 0x10 |
| POP_R0 | Pop stack to register R0 | POP_R0 | 0x11 |
| PUSH_R1 | Push register R1 onto stack | PUSH_R1 | 0x12 |
| POP_R1 | Pop stack to register R1 | POP_R1 | 0x13 |
| PUSH_R2 | Push register R2 onto stack | PUSH_R2 | 0x14 |
| POP_R2 | Pop stack to register R2 | POP_R2 | 0x15 |
| PUSH_R3 | Push register R3 onto stack | PUSH_R3 | 0x16 |
| POP_R3 | Pop stack to register R3 | POP_R3 | 0x17 |
| DUP | Duplicate top of stack | DUP | 0x18 |
| SWAP | Swap top two stack elements | SWAP | 0x19 |
| DROP | Drop top stack element | DROP | 0x1A |

### Integer Arithmetic

| Assembly | Description | Example | Opcode |
| :--- | :--- | :--- | :--- |
| ADD_INT | Add top two integers on stack | ADD_INT | 0x20 |
| SUB_INT | Subtract top two integers | SUB_INT | 0x21 |
| MUL_INT | Multiply top two integers | MUL_INT | 0x22 |
| DIV_INT | Divide top two integers | DIV_INT | 0x23 |
| MOD_INT | Modulo of top two integers | MOD_INT | 0x24 |
| NEG_INT | Negate top integer | NEG_INT | 0x25 |
| SHL_INT | Shift top integer by n bits | SHL_INT | 0x26 |
| SHR_INT | Logical shift right (zero-fill) | SHR_INT | 0x27 |
| SAR_INT | Arithmetic shift right (sign-fill) | SAR_INT | 0x28 |
| AND | Bitwise AND of top two integers | AND | 0x29 |
| OR | Bitwise OR of top two integers| OR | 0x2A |
| XOR | Bitwise XOR of top two integers| XOR | 0x2B |
| NOT | Bitwise NOT of top integer| NOT | 0x2C |

### Fixed-Point (16.16) Arithmetic

| Assembly | Description | Example | Opcode |
| :--- | :--- | :--- | :--- |
| ADD_FIXED | Add top two fixed-point numbers | ADD_FIXED | 0x40 |
| SUB_FIXED | Subtract top two fixed-point numbers | SUB_FIXED | 0x41 |
| MUL_FIXED | Multiply top two fixed-point numbers | MUL_FIXED | 0x42 |
| DIV_FIXED | Divide top two fixed-point numbers | DIV_FIXED | 0x43 |
| NEG_FIXED | Negate top fixed-point number | NEG_FIXED | 0x44 |
| INT_TO_FIXED | Convert integer to fixed-point | INT_TO_FIXED | 0x45 |
| FIXED_TO_INT | Convert fixed-point to integer | FIXED_TO_INT | 0x46 |

### Comparison

| Assembly | Description | Example | Opcode |
| :--- | :--- | :--- | :--- |
| **Integer Compare** |  |  |  |
| CMP_EQ_INT | Compare equal (int) | CMP_EQ_INT | 0x50 |
| CMP_NE_INT | Compare not equal (int) | CMP_NE_INT | 0x51 |
| CMP_LT_INT | Compare less than (int) | CMP_LT_INT | 0x52 |
| CMP_LE_INT | Compare less or equal (int) | CMP_LE_INT | 0x53 |
| CMP_GT_INT | Compare greater than (int) | CMP_GT_INT | 0x54 |
| CMP_GE_INT | Compare greater or equal (int) | CMP_GE_INT | 0x55 |
| **Fixed Compare** |  |  |  |
| CMP_EQ_FIXED | Compare equal (16.16 fixed) | CMP_EQ_FIXED | 0x56 |
| CMP_NE_FIXED | Compare not equal (fixed) | CMP_NE_FIXED | 0x57 |
| CMP_LT_FIXED | Compare less than (fixed) | CMP_LT_FIXED | 0x58 |
| CMP_LE_FIXED | Compare less or equal (fixed) | CMP_LE_FIXED | 0x59 |
| CMP_GT_FIXED | Compare greater than (fixed) | CMP_GT_FIXED | 0x5A |
| CMP_GE_FIXED | Compare greater or equal (fixed) | CMP_GE_FIXED | 0x5B |

### Branch
| Assembly | Description | Example | Opcode |
| :--- | :--- | :--- | :--- |
| JMP <addr> | Unconditional jump | JMP 0x10 | 0x60 |
| JMP_IF_TRUE <addr> | Jump if top stack != 0 | JMP_IF_TRUE 0x20 | 0x61 |
| JMP_IF_FALSE <addr> | Jump if top stack == 0 | JMP_IF_FALSE 0x30 | 0x62 |
| CALL <addr> | Call function | CALL 0x50 | 0x63 |
| RET | Return from call | RET | 0x64 |

### Extensions
| Assembly | Description | Example | Opcode |
| :--- | :--- | :--- | :--- |
| SYSCALL | Invoke system function | SYSCALL PRINT_INT | 0x70 |
| EVENT | Event & handler system | EVENT CREATE_HANDLER | 0x71 |

### System Calls
| Assembly | Description | Example | Subcode |
| :--- | :--- | :--- | :--- |
| PRINT_INT | Print integer | SYSCALL PRINT_INT | 0x01 |
| PRINT_FIXED | Print fixed-point | SYSCALL PRINT_FIXED | 0x02 |
| PRINT_STR | Print string from constant pool | SYSCALL PRINT_STR | 0x03 |


### Event
| Assembly | Description | Example | Subcode |
| :--- | :--- | :--- | :--- |
| CREATE_HANDLER | Create a new task | EVENT CREATE_HANDLER | 0x01 |
| EXIT_HANDLER | Exit current task | EVENT EXIT_HANDLER | 0x02 |
| HANDLER_SLEEP | Sleep task for ticks | EVENT HANDLER_SLEEP | 0x03 |
| SEND | Send an event | EVENT SEND 3 | 0x04 |
