class_name Task
extends RefCounted

enum State {
	RUNNING,
	SLEEPING,
	FINISHED
}

const MAX_FRAMES := 8
const LOCALS_PER_FRAME := 8
const STACK_SIZE := 8
const REGISTER_COUNT := 4

var _bytecode: PackedByteArray
var _pc: int = 0
var _sp: int = 0
var _frame: int = 0

var _state: State
var _sleep_ticks: int = 0

var _regs: PackedInt32Array = PackedInt32Array()
var _stack: PackedInt32Array = PackedInt32Array()
var _locals: PackedInt32Array = PackedInt32Array()

func _init():
	_stack.resize(STACK_SIZE)
	_regs.resize(REGISTER_COUNT)
	_locals.resize(LOCALS_PER_FRAME * MAX_FRAMES)

func set_handler(handler: Handler):
	_bytecode = handler._bytecode
	_pc = handler._entry_addr
	_sp = 0
	_frame = 0

func set_register(regs: PackedInt32Array):
	for i in range(regs.size(), REGISTER_COUNT):
		_regs[i] = regs[i]

func push_stack(value: int):
	if _sp >= _stack.size():
		push_error("Stack overflow")
		return
	_stack[_sp] = value
	_sp += 1

func pop_stack() -> int:
	if _sp <= 0:
		push_error("Stack underflow")
		return 0
	_sp -= 1
	return _stack[_sp]

func peek_stack() -> int:
	if _sp <= 0:
		push_error("Stack underflow")
		return 0
	return _stack[_sp - 1]
