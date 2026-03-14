#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Event Event;
typedef struct VM VM;
typedef struct Task Task;
typedef struct Handler Handler;
typedef struct HeapAllocator HeapAllocator;
typedef struct Allocator Allocator;
typedef struct RingBuffer_Event RingBuffer_Event;

typedef struct { uint8_t* ptr; size_t size; } __Slice_uint8_t;
typedef struct { Event* ptr; size_t size; } __Slice_Event;

typedef enum {
  TaskState_RUNNING = 0,
  TaskState_SLEEPING = 1,
  TaskState_FINISHED = 2,
} TaskState;

struct Event {
  int32_t id;
  int32_t regs[4];
};

struct Task {
  int32_t pc;
  int32_t sp;
  int32_t frame;
  TaskState state;
  int32_t sleep;
  bool active;
  int32_t stack[8];
};

struct Handler {
  int32_t event_id;
  int32_t addr;
  int32_t task;
};

struct HeapAllocator {
  uint8_t _pad;
};

struct Allocator {
  uint8_t* _ptr;
  uint32_t _capacity;
  uint32_t _cursor;
};

struct VM {
  __Slice_uint8_t _code;
  Task _tasks[8];
  Handler _handlers[8];
  int32_t _h_count;
  int32_t _max_tasks;
  int32_t _max_handlers;
  int32_t output;
};

struct RingBuffer_Event {
  __Slice_Event _buffer;
  uint32_t _head;
  uint32_t _tail;
  uint32_t _size;
};

static int32_t vm_test___wb(__Slice_uint8_t buf, int32_t i, uint8_t v);
static int32_t vm_test___wi32(__Slice_uint8_t buf, int32_t i, int32_t v);
static int32_t vm_test___wu16(__Slice_uint8_t buf, int32_t i, int32_t v);
static void vm_test___boot(__Slice_uint8_t code);
void vm_test__test_push(void);
void vm_test__test_add_int(void);
void vm_test__test_sub_int(void);
void vm_test__test_mul_int(void);
void vm_test__test_neg_int(void);
void vm_test__test_dup(void);
void vm_test__test_swap(void);
void vm_test__test_bitwise(void);
void vm_test__test_shift(void);
void vm_test__test_cmp_eq(void);
void vm_test__test_jmp_if_false(void);
void vm_test__test_call_ret(void);
void vm_test__test_signal_handler(void);
void vm_test__test_int_to_fixed(void);
void vm_test__test_fixed_to_int(void);
static void test___test_pass(void);
static void test___test_fail(__Slice_uint8_t file, uint32_t line, __Slice_uint8_t expr);
static void test___test_begin(__Slice_uint8_t name);
static void test___test_end(void);
static int32_t test___report(void);
void vm__vm_reset(void);
void vm__vm_load_sys(__Slice_uint8_t code);
void vm__vm_load_app(__Slice_uint8_t code);
void vm__vm_boot_sys(void);
void vm__vm_boot_app(void);
void vm__vm_send(int32_t id);
void vm__vm_tick(void);
void VM_init(VM* this, int32_t max_tasks, int32_t max_handlers);
void VM_reset(VM* this);
void VM_load(VM* this, __Slice_uint8_t code);
void VM_boot(VM* this);
void VM_tick_wake(VM* this);
void VM_tick_run(VM* this);
void VM_dispatch(VM* this, int32_t ev_id);
static int32_t VM__spawn(VM* this, int32_t addr);
static void VM__run(VM* this, int32_t t);
static inline int32_t VM__read_i32le(VM* this, int32_t pc);
static inline int32_t VM__read_u16(VM* this, int32_t pc);
static bool VM__execute(VM* this, int32_t t);
void console__println(void);
void console__print_str(__Slice_uint8_t s);
void console__print_bool(bool v);
void console__print_u64(uint64_t v);
void console__print_u32(uint32_t v);
void console__print_u16(uint16_t v);
void console__print_u8(uint8_t v);
void console__print_i64(int64_t v);
void console__print_i32(int32_t v);
void console__print_i16(int16_t v);
void console__print_i8(int8_t v);
void console__print_float(float v, uint32_t decimals);
void console__print_fixed(int32_t v, uint32_t decimals);
static inline void Task_reset(Task* this, int32_t addr);
static inline void Task_push(Task* this, int32_t v);
static inline int32_t Task_pop(Task* this);
static inline int32_t Task_peek(Task* this);
static inline void Task_push_bool(Task* this, bool cond);
static inline void Handler_init(Handler* this, int32_t ev, int32_t a);
void Allocator_init(Allocator* this, uint32_t capacity);
void* Allocator_allocate(Allocator* this, size_t __sizeof_T);
void Allocator_reset(Allocator* this);
static uint32_t Allocator__align(Allocator* this, uint32_t cursor, uint32_t align);
static bool Allocator__check_buffer(Allocator* this, uint32_t size);
bool RingBuffer_Event_init(RingBuffer_Event* this, Allocator* allocator, uint32_t capacity);
static inline bool RingBuffer_Event_push(RingBuffer_Event* this, Event value);
static inline bool RingBuffer_Event_pop(RingBuffer_Event* this);
static inline Event RingBuffer_Event_peek(RingBuffer_Event* this);
static inline uint32_t RingBuffer_Event_size(RingBuffer_Event* this);
static inline uint32_t RingBuffer_Event_capacity(RingBuffer_Event* this);
static inline bool RingBuffer_Event_is_empty(RingBuffer_Event* this);
static inline bool RingBuffer_Event_is_full(RingBuffer_Event* this);
__Slice_Event Allocator_allocate_array_Event(Allocator* this, uint32_t length);

static uint32_t test___succeeded = 0;
static uint32_t test___failed = 0;
static __Slice_uint8_t test___current_name = (__Slice_uint8_t){(uint8_t*)"", sizeof("") - 1};
static uint32_t test___current_failed = 0;
static uint32_t test___buf_count = 0;
static uint32_t test___buf_lines[8];
static __Slice_uint8_t test___buf_files[8];
static __Slice_uint8_t test___buf_exprs[8];
const int32_t event__EVENT_START = 1;
const int32_t event__EVENT_TICK = 2;
static uint8_t vm___alloc_buf[512];
static Allocator vm___alloc;
static RingBuffer_Event vm___events;
VM vm__sys_vm;
VM vm__app_vm;
const int32_t task__STACK_SIZE = 8;
const int32_t config__MAX_CODE = 65536;
const int32_t config__MAX_MEMORY = 65536;
const int32_t config__VM_MAX_TASKS = 8;
const int32_t config__VM_MAX_HANDLERS = 8;
const int32_t config__VM_MAX_FRAMES = 8;
const int32_t config__VM_MAX_EVENTS = 16;
const int32_t config__SYS_MAX_TASKS = 1;
const int32_t config__SYS_MAX_HANDLERS = 4;
const int32_t config__APP_MAX_TASKS = 7;
const int32_t config__APP_MAX_HANDLERS = 8;
const int32_t opcode__OP_PUSH = 0x01;
const int32_t opcode__OP_DUP = 0x18;
const int32_t opcode__OP_SWAP = 0x19;
const int32_t opcode__OP_DROP = 0x1A;
const int32_t opcode__OP_ADD_INT = 0x20;
const int32_t opcode__OP_SUB_INT = 0x21;
const int32_t opcode__OP_MUL_INT = 0x22;
const int32_t opcode__OP_DIV_INT = 0x23;
const int32_t opcode__OP_MOD_INT = 0x24;
const int32_t opcode__OP_NEG_INT = 0x25;
const int32_t opcode__OP_SHL_INT = 0x26;
const int32_t opcode__OP_SHR_INT = 0x27;
const int32_t opcode__OP_SAR_INT = 0x28;
const int32_t opcode__OP_AND = 0x29;
const int32_t opcode__OP_OR = 0x2A;
const int32_t opcode__OP_XOR = 0x2B;
const int32_t opcode__OP_NOT = 0x2C;
const int32_t opcode__OP_ADD_FIXED = 0x40;
const int32_t opcode__OP_SUB_FIXED = 0x41;
const int32_t opcode__OP_MUL_FIXED = 0x42;
const int32_t opcode__OP_DIV_FIXED = 0x43;
const int32_t opcode__OP_NEG_FIXED = 0x44;
const int32_t opcode__OP_INT_TO_FIXED = 0x45;
const int32_t opcode__OP_FIXED_TO_INT = 0x46;
const int32_t opcode__OP_CMP_EQ_INT = 0x50;
const int32_t opcode__OP_CMP_NE_INT = 0x51;
const int32_t opcode__OP_CMP_LT_INT = 0x52;
const int32_t opcode__OP_CMP_LE_INT = 0x53;
const int32_t opcode__OP_CMP_GT_INT = 0x54;
const int32_t opcode__OP_CMP_GE_INT = 0x55;
const int32_t opcode__OP_CMP_EQ_FIXED = 0x56;
const int32_t opcode__OP_CMP_NE_FIXED = 0x57;
const int32_t opcode__OP_CMP_LT_FIXED = 0x58;
const int32_t opcode__OP_CMP_LE_FIXED = 0x59;
const int32_t opcode__OP_CMP_GT_FIXED = 0x5A;
const int32_t opcode__OP_CMP_GE_FIXED = 0x5B;
const int32_t opcode__OP_JMP = 0x60;
const int32_t opcode__OP_JMP_IF_TRUE = 0x61;
const int32_t opcode__OP_JMP_IF_FALSE = 0x62;
const int32_t opcode__OP_CALL = 0x63;
const int32_t opcode__OP_RET = 0x64;
const int32_t opcode__OP_SYSCALL = 0x70;
const int32_t opcode__SYSCALL_PRINT_INT = 0x01;
const int32_t opcode__SYSCALL_PRINT_FIXED = 0x02;
const int32_t opcode__SYSCALL_PRINT_STR = 0x03;
const int32_t opcode__OP_EVENT = 0x71;
const int32_t opcode__EVENT_CREATE_HANDLER = 0x01;
const int32_t opcode__EVENT_EXIT_HANDLER = 0x02;
const int32_t opcode__EVENT_HANDLER_SLEEP = 0x03;
const int32_t opcode__EVENT_SEND = 0x04;

static int32_t vm_test___wb(__Slice_uint8_t buf, int32_t i, uint8_t v) {
  (buf.ptr[i] = v);
  return (i + 1);
}

static int32_t vm_test___wi32(__Slice_uint8_t buf, int32_t i, int32_t v) {
  (buf.ptr[i] = ((uint8_t)((v & 0xFF))));
  (buf.ptr[(i + 1)] = ((uint8_t)(((v >> 8) & 0xFF))));
  (buf.ptr[(i + 2)] = ((uint8_t)(((v >> 16) & 0xFF))));
  (buf.ptr[(i + 3)] = ((uint8_t)(((v >> 24) & 0xFF))));
  return (i + 4);
}

static int32_t vm_test___wu16(__Slice_uint8_t buf, int32_t i, int32_t v) {
  (buf.ptr[i] = ((uint8_t)((v & 0xFF))));
  (buf.ptr[(i + 1)] = ((uint8_t)(((v >> 8) & 0xFF))));
  return (i + 2);
}

static void vm_test___boot(__Slice_uint8_t code) {
  vm__vm_reset();
  vm__vm_load_sys(code);
  vm__vm_boot_sys();
}

static void test___test_pass(void) {
}

static void test___test_fail(__Slice_uint8_t file, uint32_t line, __Slice_uint8_t expr) {
  if ((test___buf_count < 8)) {
    (test___buf_files[test___buf_count] = file);
    (test___buf_lines[test___buf_count] = line);
    (test___buf_exprs[test___buf_count] = expr);
    (test___buf_count += 1);
  }
  (test___current_failed += 1);
}

static void test___test_begin(__Slice_uint8_t name) {
  (test___current_name = name);
  (test___current_failed = 0);
  (test___buf_count = 0);
}

static void test___test_end(void) {
  if ((test___current_failed == 0)) {
    console__print_str((__Slice_uint8_t){(uint8_t*)"\x1b[32mP:", sizeof("\x1b[32mP:") - 1});
    console__print_str(test___current_name);
    console__print_str((__Slice_uint8_t){(uint8_t*)"\x1b[0m", sizeof("\x1b[0m") - 1});
    console__println();
    (test___succeeded += 1);
  } else {
    console__print_str((__Slice_uint8_t){(uint8_t*)"\x1b[31mF:", sizeof("\x1b[31mF:") - 1});
    console__print_str(test___current_name);
    console__print_str((__Slice_uint8_t){(uint8_t*)"\x1b[0m", sizeof("\x1b[0m") - 1});
    console__println();
    uint32_t i = 0;
    while ((i < test___buf_count)) {
      console__print_str((__Slice_uint8_t){(uint8_t*)"  ", sizeof("  ") - 1});
      console__print_str(test___buf_files[i]);
      console__print_str((__Slice_uint8_t){(uint8_t*)":", sizeof(":") - 1});
      console__print_u32(test___buf_lines[i]);
      console__print_str((__Slice_uint8_t){(uint8_t*)": ", sizeof(": ") - 1});
      console__print_str(test___buf_exprs[i]);
      console__println();
      (i += 1);
    }
    (test___failed += 1);
  }
}

static int32_t test___report(void) {
  console__print_str((__Slice_uint8_t){(uint8_t*)"DONE ", sizeof("DONE ") - 1});
  console__print_u32(test___succeeded);
  console__print_str((__Slice_uint8_t){(uint8_t*)"/", sizeof("/") - 1});
  console__print_u32((test___succeeded + test___failed));
  console__println();
  if ((test___failed > 0)) {
    return 1;
  }
  return 0;
}

void vm__vm_reset(void) {
  Allocator_init((&vm___alloc), ((uint32_t)(512)));
  RingBuffer_Event_init((&vm___events), (&vm___alloc), ((uint32_t)(config__VM_MAX_EVENTS)));
  VM_init((&vm__sys_vm), config__SYS_MAX_TASKS, config__SYS_MAX_HANDLERS);
  VM_init((&vm__app_vm), config__APP_MAX_TASKS, config__APP_MAX_HANDLERS);
}

void vm__vm_load_sys(__Slice_uint8_t code) {
  VM_load((&vm__sys_vm), code);
}

void vm__vm_load_app(__Slice_uint8_t code) {
  VM_load((&vm__app_vm), code);
}

void vm__vm_boot_sys(void) {
  VM_boot((&vm__sys_vm));
}

void vm__vm_boot_app(void) {
  VM_boot((&vm__app_vm));
}

void vm__vm_send(int32_t id) {
  if (RingBuffer_Event_is_full((&vm___events))) {
    return;
  }
  Event ev = {0};
  (ev.id = id);
  RingBuffer_Event_push((&vm___events), ev);
}

void vm__vm_tick(void) {
  VM_tick_wake((&vm__sys_vm));
  VM_tick_wake((&vm__app_vm));
  while ((!RingBuffer_Event_is_empty((&vm___events)))) {
    Event ev = RingBuffer_Event_peek((&vm___events));
    RingBuffer_Event_pop((&vm___events));
    VM_dispatch((&vm__sys_vm), ev.id);
    VM_dispatch((&vm__app_vm), ev.id);
  }
  VM_tick_run((&vm__sys_vm));
  VM_tick_run((&vm__app_vm));
}

void VM_init(VM* this, int32_t max_tasks, int32_t max_handlers) {
  (this->_max_tasks = max_tasks);
  (this->_max_handlers = max_handlers);
  (this->_h_count = 0);
  (this->output = 0);
  (this->_code = (__Slice_uint8_t){NULL, 0});
  int32_t i = 0;
  while ((i < this->_max_tasks)) {
    (this->_tasks[i].active = false);
    i++;
  }
}

void VM_reset(VM* this) {
  (this->_h_count = 0);
  (this->output = 0);
  (this->_code = (__Slice_uint8_t){NULL, 0});
  int32_t i = 0;
  while ((i < this->_max_tasks)) {
    (this->_tasks[i].active = false);
    i++;
  }
}

void VM_load(VM* this, __Slice_uint8_t code) {
  (this->_code = code);
}

void VM_boot(VM* this) {
  int32_t t = VM__spawn(this, 0);
  if ((t >= 0)) {
    VM__run(this, t);
  }
}

void VM_tick_wake(VM* this) {
  int32_t i = 0;
  while ((i < this->_max_tasks)) {
    if ((this->_tasks[i].active && (this->_tasks[i].state == TaskState_SLEEPING))) {
      (this->_tasks[i].sleep = (this->_tasks[i].sleep - 1));
      if ((this->_tasks[i].sleep <= 0)) {
        (this->_tasks[i].state = TaskState_RUNNING);
      }
    }
    i++;
  }
}

void VM_tick_run(VM* this) {
  int32_t i = 0;
  while ((i < this->_max_tasks)) {
    if ((this->_tasks[i].active && (this->_tasks[i].state == TaskState_RUNNING))) {
      VM__run(this, i);
    }
    i++;
  }
}

void VM_dispatch(VM* this, int32_t ev_id) {
  int32_t i = 0;
  while ((i < this->_h_count)) {
    if ((this->_handlers[i].event_id == ev_id)) {
      int32_t ht = this->_handlers[i].task;
      if ((ht >= 0)) {
        (this->_tasks[ht].state = TaskState_RUNNING);
      } else {
        int32_t t = VM__spawn(this, this->_handlers[i].addr);
        if ((t >= 0)) {
          (this->_handlers[i].task = t);
        }
      }
    }
    i++;
  }
}

static int32_t VM__spawn(VM* this, int32_t addr) {
  int32_t i = 0;
  while ((i < this->_max_tasks)) {
    if ((!this->_tasks[i].active)) {
      Task_reset((&this->_tasks[i]), addr);
      return i;
    }
    i++;
  }
  return (-1);
}

static void VM__run(VM* this, int32_t t) {
  bool done = VM__execute(this, t);
  if (done) {
    int32_t i = 0;
    while ((i < this->_h_count)) {
      if ((this->_handlers[i].task == t)) {
        (this->_handlers[i].task = (-1));
      }
      i++;
    }
    (this->_tasks[t].active = false);
  }
}

static inline int32_t VM__read_i32le(VM* this, int32_t pc) {
  return (((((int32_t)(this->_code.ptr[pc])) | (((int32_t)(this->_code.ptr[(pc + 1)])) << 8)) | (((int32_t)(this->_code.ptr[(pc + 2)])) << 16)) | (((int32_t)(this->_code.ptr[(pc + 3)])) << 24));
}

static inline int32_t VM__read_u16(VM* this, int32_t pc) {
  return (((int32_t)(this->_code.ptr[pc])) | (((int32_t)(this->_code.ptr[(pc + 1)])) << 8));
}

static bool VM__execute(VM* this, int32_t t) {
  Task* task = (&this->_tasks[t]);
  int32_t pc = task->pc;
  (task->state = TaskState_RUNNING);
  while ((task->state == TaskState_RUNNING)) {
    if ((pc >= ((int32_t)(this->_code.size)))) {
      (task->state = TaskState_FINISHED);
      continue;
    }
    int32_t op = ((int32_t)(this->_code.ptr[pc]));
    pc++;
    switch (op) {
      case opcode__OP_PUSH: {
        Task_push(task, VM__read_i32le(this, pc));
        (pc += 4);
        break;
      }
      case opcode__OP_DUP: {
        Task_push(task, Task_peek(task));
        break;
      }
      case opcode__OP_SWAP: {
        int32_t top = Task_pop(task);
        int32_t sec = Task_pop(task);
        Task_push(task, top);
        Task_push(task, sec);
        break;
      }
      case opcode__OP_DROP: {
        Task_pop(task);
        break;
      }
      case opcode__OP_ADD_INT: {
        int32_t b = Task_pop(task);
        Task_push(task, (Task_pop(task) + b));
        break;
      }
      case opcode__OP_SUB_INT: {
        int32_t b = Task_pop(task);
        Task_push(task, (Task_pop(task) - b));
        break;
      }
      case opcode__OP_MUL_INT: {
        int32_t b = Task_pop(task);
        Task_push(task, (Task_pop(task) * b));
        break;
      }
      case opcode__OP_DIV_INT: {
        int32_t b = Task_pop(task);
        int32_t a = Task_pop(task);
        if ((b != 0)) {
          Task_push(task, (a / b));
        } else {
          Task_push(task, 0);
        }
        break;
      }
      case opcode__OP_MOD_INT: {
        int32_t b = Task_pop(task);
        int32_t a = Task_pop(task);
        if ((b != 0)) {
          Task_push(task, (a % b));
        } else {
          Task_push(task, 0);
        }
        break;
      }
      case opcode__OP_NEG_INT: {
        Task_push(task, (-Task_pop(task)));
        break;
      }
      case opcode__OP_SHL_INT: {
        int32_t b = Task_pop(task);
        Task_push(task, (Task_pop(task) << b));
        break;
      }
      case opcode__OP_SHR_INT: {
        int32_t b = Task_pop(task);
        Task_push(task, ((int32_t)((((uint32_t)(Task_pop(task))) >> ((uint32_t)(b))))));
        break;
      }
      case opcode__OP_SAR_INT: {
        int32_t b = Task_pop(task);
        Task_push(task, (Task_pop(task) >> b));
        break;
      }
      case opcode__OP_AND: {
        int32_t b = Task_pop(task);
        Task_push(task, (Task_pop(task) & b));
        break;
      }
      case opcode__OP_OR: {
        int32_t b = Task_pop(task);
        Task_push(task, (Task_pop(task) | b));
        break;
      }
      case opcode__OP_XOR: {
        int32_t b = Task_pop(task);
        Task_push(task, (Task_pop(task) ^ b));
        break;
      }
      case opcode__OP_NOT: {
        Task_push(task, (~Task_pop(task)));
        break;
      }
      case opcode__OP_ADD_FIXED: {
        int32_t b = Task_pop(task);
        Task_push(task, (Task_pop(task) + b));
        break;
      }
      case opcode__OP_SUB_FIXED: {
        int32_t b = Task_pop(task);
        Task_push(task, (Task_pop(task) - b));
        break;
      }
      case opcode__OP_MUL_FIXED: {
        int32_t b = Task_pop(task);
        int32_t a = Task_pop(task);
        Task_push(task, ((int32_t)(((((int64_t)(a)) * ((int64_t)(b))) >> 16))));
        break;
      }
      case opcode__OP_DIV_FIXED: {
        int32_t b = Task_pop(task);
        int32_t a = Task_pop(task);
        if ((b != 0)) {
          Task_push(task, ((int32_t)(((((int64_t)(a)) << 16) / ((int64_t)(b))))));
        } else {
          Task_push(task, 0);
        }
        break;
      }
      case opcode__OP_NEG_FIXED: {
        Task_push(task, (-Task_pop(task)));
        break;
      }
      case opcode__OP_INT_TO_FIXED: {
        Task_push(task, (Task_pop(task) << 16));
        break;
      }
      case opcode__OP_FIXED_TO_INT: {
        Task_push(task, (Task_pop(task) >> 16));
        break;
      }
      case opcode__OP_CMP_EQ_INT: {
        int32_t b = Task_pop(task);
        Task_push_bool(task, (Task_pop(task) == b));
        break;
      }
      case opcode__OP_CMP_NE_INT: {
        int32_t b = Task_pop(task);
        Task_push_bool(task, (Task_pop(task) != b));
        break;
      }
      case opcode__OP_CMP_LT_INT: {
        int32_t b = Task_pop(task);
        Task_push_bool(task, (Task_pop(task) < b));
        break;
      }
      case opcode__OP_CMP_LE_INT: {
        int32_t b = Task_pop(task);
        Task_push_bool(task, (Task_pop(task) <= b));
        break;
      }
      case opcode__OP_CMP_GT_INT: {
        int32_t b = Task_pop(task);
        Task_push_bool(task, (Task_pop(task) > b));
        break;
      }
      case opcode__OP_CMP_GE_INT: {
        int32_t b = Task_pop(task);
        Task_push_bool(task, (Task_pop(task) >= b));
        break;
      }
      case opcode__OP_CMP_EQ_FIXED: {
        int32_t b = Task_pop(task);
        Task_push_bool(task, (Task_pop(task) == b));
        break;
      }
      case opcode__OP_CMP_NE_FIXED: {
        int32_t b = Task_pop(task);
        Task_push_bool(task, (Task_pop(task) != b));
        break;
      }
      case opcode__OP_CMP_LT_FIXED: {
        int32_t b = Task_pop(task);
        Task_push_bool(task, (Task_pop(task) < b));
        break;
      }
      case opcode__OP_CMP_LE_FIXED: {
        int32_t b = Task_pop(task);
        Task_push_bool(task, (Task_pop(task) <= b));
        break;
      }
      case opcode__OP_CMP_GT_FIXED: {
        int32_t b = Task_pop(task);
        Task_push_bool(task, (Task_pop(task) > b));
        break;
      }
      case opcode__OP_CMP_GE_FIXED: {
        int32_t b = Task_pop(task);
        Task_push_bool(task, (Task_pop(task) >= b));
        break;
      }
      case opcode__OP_JMP: {
        (pc = VM__read_u16(this, pc));
        break;
      }
      case opcode__OP_JMP_IF_TRUE: {
        int32_t addr = VM__read_u16(this, pc);
        (pc += 2);
        if ((Task_pop(task) != 0)) {
          (pc = addr);
        }
        break;
      }
      case opcode__OP_JMP_IF_FALSE: {
        int32_t addr = VM__read_u16(this, pc);
        (pc += 2);
        if ((Task_pop(task) == 0)) {
          (pc = addr);
        }
        break;
      }
      case opcode__OP_CALL: {
        int32_t addr = VM__read_u16(this, pc);
        (pc += 2);
        (task->frame = (task->frame + 1));
        if ((task->frame >= config__VM_MAX_FRAMES)) {
          (task->state = TaskState_FINISHED);
        } else {
          int32_t argc = Task_pop(task);
          int32_t j = 0;
          while ((j < argc)) {
            Task_pop(task);
            j++;
          }
          Task_push(task, pc);
          (pc = addr);
        }
        break;
      }
      case opcode__OP_RET: {
        int32_t retval = Task_pop(task);
        int32_t retaddr = Task_pop(task);
        (task->frame = (task->frame - 1));
        (pc = retaddr);
        Task_push(task, retval);
        break;
      }
      case opcode__OP_SYSCALL: {
        int32_t sub = ((int32_t)(this->_code.ptr[pc]));
        pc++;
        switch (sub) {
          case opcode__SYSCALL_PRINT_INT: {
            (this->output = Task_pop(task));
            break;
          }
          case opcode__SYSCALL_PRINT_FIXED: {
            (this->output = Task_pop(task));
            break;
          }
          default: {
            (task->state = TaskState_FINISHED);
            break;
          }
        }
        break;
      }
      case opcode__OP_EVENT: {
        int32_t sub = ((int32_t)(this->_code.ptr[pc]));
        pc++;
        switch (sub) {
          case opcode__EVENT_CREATE_HANDLER: {
            int32_t ha = Task_pop(task);
            int32_t he = Task_pop(task);
            if ((this->_h_count < this->_max_handlers)) {
              Handler_init((&this->_handlers[this->_h_count]), he, ha);
              this->_h_count++;
            }
            break;
          }
          case opcode__EVENT_EXIT_HANDLER: {
            (task->state = TaskState_FINISHED);
            break;
          }
          case opcode__EVENT_HANDLER_SLEEP: {
            (task->sleep = Task_pop(task));
            (task->state = TaskState_SLEEPING);
            break;
          }
          case opcode__EVENT_SEND: {
            vm__vm_send(Task_pop(task));
            break;
          }
          default: {
            (task->state = TaskState_FINISHED);
            break;
          }
        }
        break;
      }
      default: {
        (task->state = TaskState_FINISHED);
        break;
      }
    }
  }
  (task->pc = pc);
  return (task->state == TaskState_FINISHED);
}

void console__println(void) {
  putchar(10);
}

void console__print_str(__Slice_uint8_t s) {
  uint32_t i = 0;
  while ((i < s.size)) {
    putchar(s.ptr[i]);
    (i += 1);
  }
}

void console__print_bool(bool v) {
  if (v) {
    putchar('t');
    putchar('r');
    putchar('u');
    putchar('e');
  } else {
    putchar('f');
    putchar('a');
    putchar('l');
    putchar('s');
    putchar('e');
  }
}

void console__print_u64(uint64_t v) {
  uint8_t buf[20];
  int32_t i = 19;
  if ((v == 0)) {
    putchar('0');
    return;
  }
  while ((v > 0)) {
    (buf[i] = ((uint8_t)(((v % 10) + 48))));
    (v = (v / 10));
    (i -= 1);
  }
  int32_t j = (i + 1);
  while ((j < 20)) {
    putchar(buf[j]);
    (j += 1);
  }
}

void console__print_u32(uint32_t v) {
  console__print_u64(((uint64_t)(v)));
}

void console__print_u16(uint16_t v) {
  console__print_u64(((uint64_t)(v)));
}

void console__print_u8(uint8_t v) {
  console__print_u64(((uint64_t)(v)));
}

void console__print_i64(int64_t v) {
  if ((v < 0)) {
    putchar('-');
    console__print_u64(((uint64_t)((((int64_t)(0)) - v))));
  } else {
    console__print_u64(((uint64_t)(v)));
  }
}

void console__print_i32(int32_t v) {
  console__print_i64(((int64_t)(v)));
}

void console__print_i16(int16_t v) {
  console__print_i64(((int64_t)(v)));
}

void console__print_i8(int8_t v) {
  console__print_i64(((int64_t)(v)));
}

void console__print_float(float v, uint32_t decimals) {
  float abs = v;
  if ((v < 0.0f)) {
    putchar('-');
    (abs = (0.0f - v));
  }
  int32_t int_part = ((int32_t)(abs));
  console__print_i32(int_part);
  if ((decimals > 0)) {
    putchar('.');
    float frac = (abs - ((float)(int_part)));
    uint32_t i = 0;
    while ((i < decimals)) {
      (frac = (frac * 10.0f));
      int32_t digit = ((int32_t)(frac));
      putchar(((uint8_t)((digit + 48))));
      (frac = (frac - ((float)(digit))));
      (i += 1);
    }
  }
}

void console__print_fixed(int32_t v, uint32_t decimals) {
  uint32_t abs = 0;
  if ((v < 0)) {
    putchar('-');
    (abs = ((uint32_t)((0 - v))));
  } else {
    (abs = ((uint32_t)(v)));
  }
  console__print_u32((abs >> 16));
  if ((decimals > 0)) {
    putchar('.');
    uint32_t frac = (abs & 0xFFFF);
    uint32_t i = 0;
    while ((i < decimals)) {
      (frac *= 10);
      putchar(((uint8_t)(((frac >> 16) + 48))));
      (frac = (frac & 0xFFFF));
      (i += 1);
    }
  }
}

static inline void Task_reset(Task* this, int32_t addr) {
  (this->pc = addr);
  (this->sp = 0);
  (this->frame = 0);
  (this->state = TaskState_RUNNING);
  (this->sleep = 0);
  (this->active = true);
}

static inline void Task_push(Task* this, int32_t v) {
  if ((this->sp >= task__STACK_SIZE)) {
    return;
  }
  (this->stack[this->sp] = v);
  (this->sp = (this->sp + 1));
}

static inline int32_t Task_pop(Task* this) {
  if ((this->sp <= 0)) {
    return 0;
  }
  (this->sp = (this->sp - 1));
  return this->stack[this->sp];
}

static inline int32_t Task_peek(Task* this) {
  if ((this->sp <= 0)) {
    return 0;
  }
  return this->stack[(this->sp - 1)];
}

static inline void Task_push_bool(Task* this, bool cond) {
  if (cond) {
    Task_push(this, 1);
  } else {
    Task_push(this, 0);
  }
}

static inline void Handler_init(Handler* this, int32_t ev, int32_t a) {
  (this->event_id = ev);
  (this->addr = a);
  (this->task = (-1));
}

void Allocator_init(Allocator* this, uint32_t capacity) {
  (this->_ptr = malloc(capacity));
  (this->_capacity = capacity);
}

void* Allocator_allocate(Allocator* this, size_t __sizeof_T) {
  uint64_t size = __sizeof_T;
  if (Allocator__check_buffer(this, size)) {
    void* ptr = (void*)((&this->_ptr[this->_cursor]));
    (this->_cursor = Allocator__align(this, (this->_cursor + size), 8));
    return ptr;
  }
  return NULL;
}

void Allocator_reset(Allocator* this) {
  (this->_cursor = 0);
}

static uint32_t Allocator__align(Allocator* this, uint32_t cursor, uint32_t align) {
  return (((cursor + align) - 1) & (~(align - 1)));
}

static bool Allocator__check_buffer(Allocator* this, uint32_t size) {
  uint32_t aligned = Allocator__align(this, (this->_cursor + size), 8);
  if ((aligned > this->_capacity)) {
    (this->_capacity = (this->_capacity * 2));
    (this->_ptr = realloc(this->_ptr, this->_capacity));
    if ((this->_ptr == NULL)) {
      return false;
    }
  }
  return true;
}

bool RingBuffer_Event_init(RingBuffer_Event* this, Allocator* allocator, uint32_t capacity) {
  (this->_buffer = Allocator_allocate_array_Event(allocator, capacity));
  if ((this->_buffer.size == 0)) {
    return false;
  }
  return true;
}

static inline bool RingBuffer_Event_push(RingBuffer_Event* this, Event value) {
  if ((this->_size >= this->_buffer.size)) {
    return false;
  }
  (this->_buffer.ptr[this->_tail] = value);
  (this->_tail = ((this->_tail + 1) % this->_buffer.size));
  (this->_size += 1);
  return true;
}

static inline bool RingBuffer_Event_pop(RingBuffer_Event* this) {
  if ((this->_size == 0)) {
    return false;
  }
  (this->_head = ((this->_head + 1) % this->_buffer.size));
  (this->_size -= 1);
  return true;
}

static inline Event RingBuffer_Event_peek(RingBuffer_Event* this) {
  return this->_buffer.ptr[this->_head];
}

static inline uint32_t RingBuffer_Event_size(RingBuffer_Event* this) {
  return this->_size;
}

static inline uint32_t RingBuffer_Event_capacity(RingBuffer_Event* this) {
  return this->_buffer.size;
}

static inline bool RingBuffer_Event_is_empty(RingBuffer_Event* this) {
  return (this->_size == 0);
}

static inline bool RingBuffer_Event_is_full(RingBuffer_Event* this) {
  return (this->_size >= this->_buffer.size);
}

__Slice_Event Allocator_allocate_array_Event(Allocator* this, uint32_t length) {
  uint64_t size = (sizeof(Event) * length);
  if (Allocator__check_buffer(this, size)) {
    Event* ptr = ((Event*)((&this->_ptr[this->_cursor])));
    (this->_cursor = Allocator__align(this, (this->_cursor + size), 8));
    return (__Slice_Event){ptr, length};
  }
  return (__Slice_Event){NULL, 0};
}

void vm_test__test_push(void) {
  uint8_t raw[16];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(16))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 42));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  vm_test___boot((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  if (!((vm__sys_vm.output == 42))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 51, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == 42)", 27});
  } else {
    test___test_pass();
  }
}

void vm_test__test_add_int(void) {
  uint8_t raw[32];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(32))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 10));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 3));
  (i = vm_test___wb(code, i, ((uint8_t)(0x20))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  vm_test___boot((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  if (!((vm__sys_vm.output == 13))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 70, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == 13)", 27});
  } else {
    test___test_pass();
  }
}

void vm_test__test_sub_int(void) {
  uint8_t raw[32];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(32))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 10));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 3));
  (i = vm_test___wb(code, i, ((uint8_t)(0x21))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  vm_test___boot((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  if (!((vm__sys_vm.output == 7))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 87, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == 7)", 26});
  } else {
    test___test_pass();
  }
}

void vm_test__test_mul_int(void) {
  uint8_t raw[32];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(32))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 6));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 7));
  (i = vm_test___wb(code, i, ((uint8_t)(0x22))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  vm_test___boot((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  if (!((vm__sys_vm.output == 42))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 104, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == 42)", 27});
  } else {
    test___test_pass();
  }
}

void vm_test__test_neg_int(void) {
  uint8_t raw[32];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(32))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 5));
  (i = vm_test___wb(code, i, ((uint8_t)(0x25))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  vm_test___boot((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  if (!((vm__sys_vm.output == (-5)))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 119, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == -5)", 27});
  } else {
    test___test_pass();
  }
}

void vm_test__test_dup(void) {
  uint8_t raw[32];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(32))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 7));
  (i = vm_test___wb(code, i, ((uint8_t)(0x18))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x20))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  vm_test___boot((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  if (!((vm__sys_vm.output == 14))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 137, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == 14)", 27});
  } else {
    test___test_pass();
  }
}

void vm_test__test_swap(void) {
  uint8_t raw[32];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(32))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 10));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 3));
  (i = vm_test___wb(code, i, ((uint8_t)(0x19))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x21))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  vm_test___boot((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  if (!((vm__sys_vm.output == (-7)))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 155, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == -7)", 27});
  } else {
    test___test_pass();
  }
}

void vm_test__test_bitwise(void) {
  uint8_t raw[32];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(32))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 0xFF));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 0x0F));
  (i = vm_test___wb(code, i, ((uint8_t)(0x29))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  vm_test___boot((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  if (!((vm__sys_vm.output == 0x0F))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 174, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == 0x0F)", 29});
  } else {
    test___test_pass();
  }
}

void vm_test__test_shift(void) {
  uint8_t raw[32];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(32))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 1));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 4));
  (i = vm_test___wb(code, i, ((uint8_t)(0x26))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  vm_test___boot((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  if (!((vm__sys_vm.output == 16))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 191, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == 16)", 27});
  } else {
    test___test_pass();
  }
}

void vm_test__test_cmp_eq(void) {
  uint8_t raw[32];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(32))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 5));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 5));
  (i = vm_test___wb(code, i, ((uint8_t)(0x50))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  vm_test___boot((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  if (!((vm__sys_vm.output == 1))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 210, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == 1)", 26});
  } else {
    test___test_pass();
  }
}

void vm_test__test_jmp_if_false(void) {
  uint8_t raw[32];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(32))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 0));
  (i = vm_test___wb(code, i, ((uint8_t)(0x62))));
  (i = vm_test___wu16(code, i, 17));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 1));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 99));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  vm_test___boot((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  if (!((vm__sys_vm.output == 99))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 244, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == 99)", 27});
  } else {
    test___test_pass();
  }
}

void vm_test__test_call_ret(void) {
  uint8_t raw[32];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(32))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 0));
  (i = vm_test___wb(code, i, ((uint8_t)(0x63))));
  (i = vm_test___wu16(code, i, 15));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  while ((i < 15)) {
    (i = vm_test___wb(code, i, ((uint8_t)(0x1A))));
  }
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 42));
  (i = vm_test___wb(code, i, ((uint8_t)(0x64))));
  vm_test___boot((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  if (!((vm__sys_vm.output == 42))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 274, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == 42)", 27});
  } else {
    test___test_pass();
  }
}

void vm_test__test_signal_handler(void) {
  uint8_t raw[64];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(64))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, event__EVENT_START));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 17));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  while ((i < 17)) {
    (i = vm_test___wb(code, i, ((uint8_t)(0x1A))));
  }
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 77));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  vm__vm_reset();
  vm__vm_load_sys((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  vm__vm_boot_sys();
  vm__vm_send(event__EVENT_START);
  vm__vm_tick();
  if (!((vm__sys_vm.output == 77))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 314, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == 77)", 27});
  } else {
    test___test_pass();
  }
}

void vm_test__test_int_to_fixed(void) {
  uint8_t raw[32];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(32))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 3));
  (i = vm_test___wb(code, i, ((uint8_t)(0x45))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  vm_test___boot((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  if (!((vm__sys_vm.output == 196608))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 332, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == 196608)", 31});
  } else {
    test___test_pass();
  }
}

void vm_test__test_fixed_to_int(void) {
  uint8_t raw[32];
  __Slice_uint8_t code = (__Slice_uint8_t){(&raw[0]), ((uint32_t)(32))};
  int32_t i = 0;
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wi32(code, i, 196608));
  (i = vm_test___wb(code, i, ((uint8_t)(0x46))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x70))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x01))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x71))));
  (i = vm_test___wb(code, i, ((uint8_t)(0x02))));
  vm_test___boot((__Slice_uint8_t){(&raw[0]), ((uint32_t)(i))});
  if (!((vm__sys_vm.output == 3))) {
    test___test_fail((__Slice_uint8_t){(uint8_t*)"/Users/sang/Dev/panda-io/pico-panda/src/vm_test.mpd", 51}, 348, (__Slice_uint8_t){(uint8_t*)"assert(sys_vm.output == 3)", 26});
  } else {
    test___test_pass();
  }
}

int main(void) {
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_push", 9});
    vm_test__test_push();
    test___test_end();
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_add_int", 12});
    vm_test__test_add_int();
    test___test_end();
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_sub_int", 12});
    vm_test__test_sub_int();
    test___test_end();
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_mul_int", 12});
    vm_test__test_mul_int();
    test___test_end();
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_neg_int", 12});
    vm_test__test_neg_int();
    test___test_end();
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_dup", 8});
    vm_test__test_dup();
    test___test_end();
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_swap", 9});
    vm_test__test_swap();
    test___test_end();
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_bitwise", 12});
    vm_test__test_bitwise();
    test___test_end();
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_shift", 10});
    vm_test__test_shift();
    test___test_end();
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_cmp_eq", 11});
    vm_test__test_cmp_eq();
    test___test_end();
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_jmp_if_false", 17});
    vm_test__test_jmp_if_false();
    test___test_end();
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_call_ret", 13});
    vm_test__test_call_ret();
    test___test_end();
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_signal_handler", 19});
    vm_test__test_signal_handler();
    test___test_end();
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_int_to_fixed", 17});
    vm_test__test_int_to_fixed();
    test___test_end();
    test___test_begin((__Slice_uint8_t){(uint8_t*)"test_fixed_to_int", 17});
    vm_test__test_fixed_to_int();
    test___test_end();
    return test___report();
}

