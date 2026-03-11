class_name VM
extends RefCounted

const MEMORY_SIZE := 256

var _suspended_tasks: Array[Task] = []
var _task_pool: Array[Task] = []
var _event_queue: Array[Event] = []
var _event_pool: Array[Event] = []
var _event_registers: PackedInt32Array = PackedInt32Array()
var _handlers: Dictionary = {} # event_id -> Handler
var _globals: PackedInt32Array = PackedInt32Array()
var _console: Console = Console.new()

func _init():
	_event_registers.resize(Task.REGISTER_COUNT)
	_globals.resize(MEMORY_SIZE)
	reset()

func reset():
	for task in _suspended_tasks:
		_recycle_task(task)
	_suspended_tasks.clear()
	for event in _event_queue:
		_recycle_event(event)
	_event_queue.clear()
	_event_registers.fill(0)
	_handlers.clear()
	_globals.fill(0)

func add_handler(event_id: int, handler: Handler) -> void:
	if not _handlers.has(event_id):
		_handlers[event_id] = []
	_handlers[event_id].append(handler)

func add_event_queue(event: Event) -> void:
	_event_queue.append(event)

func tick():
	for task in _suspended_tasks:
		task._sleep_ticks -= 1
		if task._sleep_ticks <= 0:
			task._state = Task.State.RUNNING
			var event = _get_or_create_event()
			_event_registers.fill(0)
			event.set_data(Event.EVENT_TICK, _event_registers)
			_handle(task._suspended_task, event)
			_recycle_event(event)

	for event in _event_queue:
		_dispatch_event(event)
		_recycle_event(event)

func _get_or_create_task() -> Task:
	if _task_pool.size() > 0:
		return _task_pool.pop_back()
	
	print("Creating new task")
	return Task.new()

func _recycle_task(task: Task) -> void:
	_task_pool.append(task)

func _get_or_create_event() -> Event:
	if _event_pool.size() > 0:
		return _event_pool.pop_back()
	
	print("Creating new event")
	return Event.new()

func _recycle_event(event: Event) -> void:
	_event_pool.append(event)

func _dispatch_event(event: Event) -> void:
	if _handlers.has(event._event_id):
		var handlers = _handlers[event._event_id]
		for handler in handlers:
			if handler._suspended_task != null:
				_handle(handler, event)
	
	_recycle_event(event)
			
func _handle(handler: Handler, event: Event) -> void:
	var task
	var from_suspended_task = handler._suspended_task != null
	if from_suspended_task:
		task = handler._suspended_task
	else:
		task = _get_or_create_task()
		task.set_handler(handler)
	task.set_register(event._regs)

	var task_finished = _execute(task)
	if task_finished:
		_recycle_task(task)
		handler._suspended_task = null
		if from_suspended_task:
			_suspended_tasks.erase(task)
	else:
		handler._suspended_task = task
		if not from_suspended_task:
			_suspended_tasks.append(task)

func _execute(task: Task) -> bool:
	task._state = Task.State.RUNNING

	var code = task._bytecode
	var pc = task._pc
	var registers = task._regs
	var locals = task._locals

	while task._state == Task.State.RUNNING:
		if pc >= code.size():
			task._state = Task.State.FINISHED
			continue
			
		var opcode = code[pc]; pc += 1
		
		match opcode:
			# Data Operations
			0x01: # PUSH <val>
				var value = int(code[pc]) | int(code[pc + 1]) << 8 | int(code[pc + 2]) << 16 | int(code[pc + 3]) << 24
				pc += 4
				task.push_stack(value)
			0x02: # LOAD_GLOBAL <idx>
				var idx = code[pc]; pc += 1
				task.push_stack(_globals[idx])
			0x03: # STORE_GLOBAL <idx>
				var idx = code[pc]; pc += 1
				_globals[idx] = task.pop_stack()
			0x04: # LOAD_LOCAL <idx>
				var idx = code[pc]; pc += 1
				task.push_stack(locals[idx + task._frame * task.LOCALS_PER_FRAME])
			0x05: # STORE_LOCAL <idx>
				var idx = code[pc]; pc += 1
				locals[idx + task._frame * task.LOCALS_PER_FRAME] = task.pop_stack()
			
			# Stack Operations
			0x10: # PUSH_R0
				task.push_stack(registers[0])
			0x11: # POP_R0
				registers[0] = task.pop_stack()
			0x12: # PUSH_R1
				task.push_stack(registers[1])
			0x13: # POP_R1
				registers[1] = task.pop_stack()
			0x14: # PUSH_R2
				task.push_stack(registers[2])
			0x15: # POP_R2
				registers[2] = task.pop_stack()
			0x16: # PUSH_R3
				task.push_stack(registers[3])
			0x17: # POP_R3
				registers[3] = task.pop_stack()
			0x18: # DUP
				task.push_stack(task.peek_stack())
			0x19: # SWAP
				var top = task.pop_stack()
				var second = task.pop_stack()
				task.push_stack(top)
				task.push_stack(second)
			0x1A: # DROP
				task.pop_stack()
			
			# Integer Arithmetic
			0x20: # ADD_INT
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(a + b)
			0x21: # SUB_INT
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(a - b)
			0x22: # MUL_INT
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(a * b)
			0x23: # DIV_INT
				var b = task.pop_stack()
				var a = task.pop_stack()
				@warning_ignore("integer_division")
				task.push_stack(a / b) # integer division
			0x24: # MOD_INT
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(a % b)
			0x25: # NEG_INT
				var a = task.pop_stack()
				task.push_stack(-a)
			0x29: # AND
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(a & b)
			0x2A: # OR
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(a | b)
			0x2B: # XOR
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(a ^ b)
			0x2C: # NOT
				var a = task.pop_stack()
				task.push_stack(~a)
			
			# Fixed-Point (16.16) Arithmetic
			0x40: # ADD_FIXED
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(a + b)
			0x41: # SUB_FIXED
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(a - b)
			0x42: # MUL_FIXED
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack((a * b) >> 16)
			0x43: # DIV_FIXED
				var b = task.pop_stack()
				var a = task.pop_stack()
				@warning_ignore("integer_division")
				task.push_stack((a << 16) / b)
			0x44: # NEG_FIXED
				var a = task.pop_stack()
				task.push_stack(-a)
			0x45: # INT_TO_FIXED
				var a = task.pop_stack()
				task.push_stack(a << 16)
			0x46: # FIXED_TO_INT
				var a = task.pop_stack()
				task.push_stack(a >> 16)
			
			# Integer Compare
			0x50: # CMP_EQ_INT
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(1 if a == b else 0)
			0x51: # CMP_NE_INT
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(1 if a != b else 0)
			0x52: # CMP_LT_INT
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(1 if a < b else 0)
			0x53: # CMP_LE_INT
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(1 if a <= b else 0)
			0x54: # CMP_GT_INT
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(1 if a > b else 0)
			0x55: # CMP_GE_INT
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(1 if a >= b else 0)

			# Fixed Compare (16.16)
			0x56: # CMP_EQ_FIXED
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(1 if a == b else 0)

			0x57: # CMP_NE_FIXED
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(1 if a != b else 0)

			0x58: # CMP_LT_FIXED
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(1 if a < b else 0)

			0x59: # CMP_LE_FIXED
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(1 if a <= b else 0)

			0x5A: # CMP_GT_FIXED
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(1 if a > b else 0)

			0x5B: # CMP_GE_FIXED
				var b = task.pop_stack()
				var a = task.pop_stack()
				task.push_stack(1 if a >= b else 0)
				
			# Comparison / Branch
			0x60: # JMP <addr>
				var addr = int(code[pc]) | int(code[pc + 1]) << 8; pc += 2
				pc = addr
			0x61: # JMP_IF_TRUE <addr>
				var addr = int(code[pc]) | int(code[pc + 1]) << 8; pc += 2
				var cond = task.pop_stack()
				if cond != 0:
					pc = addr
			0x62: # JMP_IF_FALSE <addr>
				var addr = int(code[pc]) | int(code[pc + 1]) << 8; pc += 2
				var cond = task.pop_stack()
				if cond == 0:
					pc = addr
			0x63: # CALL <addr>
				var addr = int(code[pc]) | int(code[pc + 1]) << 8; pc += 2
				task._frame += 1
				if task._frame >= task.MAX_FRAMES:
					push_error("Max call frame depth exceeded")
					task._state = Task.State.FINISHED
				var arg_count = task.pop_stack()
				for i in range(arg_count):
					# args are passed to local variables of the callee in reverse order
					var arg_value = task.pop_stack()
					locals[(task._frame * task.LOCALS_PER_FRAME) + (arg_count - 1 - i)] = arg_value
				task.push_stack(pc)
				pc = addr
			0x64: # RET
				var ret_value = task.pop_stack()
				pc = task.pop_stack()
				task._frame -= 1
				task.push_stack(ret_value)

			# System Calls
			0x70: # SYSCALL <idx>
				var sub_code = code[pc]; pc += 1
				match sub_code:
					0x01: # PRINT_INT
						var value = task.pop_stack()
						_console.output(value)
					0x02: # PRINT_FIXED
						var value = task.pop_stack()
						_console.output(float(value) / 65536.0)
					0x03: # PRINT_STR
						var addr = task.pop_stack()
						var str_len = int(code[addr]) | int(code[addr + 1]) << 8
						var str_bytes = code.slice(addr + 2, addr + 2 + str_len)
						var string = str_bytes.get_string_from_utf8()
						_console.output(string)
					_: # unknown syscall
						push_error("Unknown syscall %s" % sub_code)
						task._state = Task.State.FINISHED

			# Event System
			0x71: # EVENT
				var sub_code = code[pc]; pc += 1
				match sub_code:
					0x01: # CREATE
						var entry_addr = task.pop_stack()
						var event_id = task.pop_stack()
						var handler = Handler.new()
						handler._bytecode = code
						handler._entry_addr = entry_addr
						add_handler(event_id, handler)
					0x02: # EXIT
						task._state = Task.State.FINISHED
					0x03: # SLEEP
						var ticks = task.pop_stack()
						task._pc = pc
						task.sleep_ticks = ticks
						task._state = Task.State.SLEEPING
					0x05: # SEND_EVENT
						var event_id = task.pop_stack()
						var event = _get_or_create_event()
						event.set_data(event_id, registers)
						add_event_queue(event)
					_: # unknown task subcode
						push_error("Unknown task subcode %s" % sub_code)
						task._state = Task.State.FINISHED

			# TODO: other extensions should be registered dynamically

			_: # unknown opcode
				push_error("Unknown opcode %s" % opcode)
				task._state = Task.State.FINISHED

	return task._state == Task.State.FINISHED
