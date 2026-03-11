class_name Assembler
extends RefCounted

const OPCODES := {
	# Data
	"PUSH": 0x01,
	"LOAD_GLOBAL": 0x02,
	"STORE_GLOBAL": 0x03,
	"LOAD_LOCAL": 0x04,
	"STORE_LOCAL": 0x05,
	# Pseudo instructions
	"PUSH_FLOAT": 0x01,
	"PUSH_STR": 0x01,

	# Stack / Registers
	"PUSH_R0": 0x10,
	"POP_R0": 0x11,
	"PUSH_R1": 0x12,
	"POP_R1": 0x13,
	"PUSH_R2": 0x14,
	"POP_R2": 0x15,
	"PUSH_R3": 0x16,
	"POP_R3": 0x17,
	"DUP": 0x18,
	"SWAP": 0x19,
	"DROP": 0x1A,

	# Integer
	"ADD_INT": 0x20,
	"SUB_INT": 0x21,
	"MUL_INT": 0x22,
	"DIV_INT": 0x23,
	"MOD_INT": 0x24,
	"NEG_INT": 0x25,
	"SHL_INT": 0x26,
	"SHR_INT": 0x27,
	"SAR_INT": 0x28,
	"AND": 0x29,
	"OR": 0x2A,
	"XOR": 0x2B,
	"NOT": 0x2C,

	# Fixed
	"ADD_FIXED": 0x40,
	"SUB_FIXED": 0x41,
	"MUL_FIXED": 0x42,
	"DIV_FIXED": 0x43,
	"NEG_FIXED": 0x44,
	"INT_TO_FIXED": 0x45,
	"FIXED_TO_INT": 0x46,

	# Compare
	"CMP_EQ_INT": 0x50,
	"CMP_NE_INT": 0x51,
	"CMP_LT_INT": 0x52,
	"CMP_LE_INT": 0x53,
	"CMP_GT_INT": 0x54,
	"CMP_GE_INT": 0x55,
	"CMP_EQ_FIXED": 0x56,
	"CMP_NE_FIXED": 0x57,
	"CMP_LT_FIXED": 0x58,
	"CMP_LE_FIXED": 0x59,
	"CMP_GT_FIXED": 0x5A,
	"CMP_GE_FIXED": 0x5B,

	# Branch
	"JMP": 0x60,
	"JMP_IF_TRUE": 0x61,
	"JMP_IF_FALSE": 0x62,
	"CALL": 0x63,
	"RET": 0x64,
}

const SYSCALL_OPCODES := {
	"PRINT_INT": 0x01,
	"PRINT_FIXED": 0x02,
	"PRINT_STR": 0x03,
}

const EVENT_OPCODES := {
	"CREATE_HANDLER": 0x01,
	"EXIT_HANDLER": 0x02,
	"HANDLER_SLEEP": 0x03,
	"SEND": 0x04,
}

var _constant_pool: PackedByteArray = PackedByteArray()
var _constant_offset: int = 0
var _string_offsets: Dictionary = {} # string -> offset

var _extensions := {
	"SYSCALL": 0x70,
	"EVENT": 0x71,
}

var _extension_subcodes := {
	"SYSCALL": SYSCALL_OPCODES,
	"EVENT": EVENT_OPCODES,
}

func register_extension(name: String, opcode: int, subcodes: Dictionary) -> void:
	if _extensions.has(name):
		push_error("Extension %s already exists" % name)
		return
	
	_extensions[name] = opcode
	_extension_subcodes[name] = subcodes

func assemble(source: String) -> PackedByteArray:
	_constant_pool.clear()
	_constant_offset = 0
	_string_offsets.clear()

	var lines = source.split("\n")
	var labels = _first_pass(lines)

	var bytecode := PackedByteArray()

	for raw_line in lines:
		var line = raw_line.strip_edges()
		if line == "" or line.begins_with(";") or line.ends_with(":"):
			continue

		var parts = line.split(" ", false)
		var inst = parts[0]

		var opcode = OPCODES.get(inst, -1)
		if opcode == -1:
			if not _extensions.has(inst):
				push_error("Unknown instruction: %s" % inst)
				return PackedByteArray()
			opcode = _extensions[inst]

		bytecode.append(opcode)

		match inst:
			"PUSH":
				var v = int(parts[1])
				bytecode.append(v & 0xFF)
				bytecode.append((v >> 8) & 0xFF)
				bytecode.append((v >> 16) & 0xFF)
				bytecode.append((v >> 24) & 0xFF)

			"PUSH_STR":
				var first_quote = line.find('"')
				var last_quote = line.rfind('"')
				if first_quote == -1 or last_quote <= first_quote:
					push_error("Invalid string literal: %s" % line)
					return PackedByteArray()
				var string_val = line.substr(first_quote + 1, last_quote - first_quote - 1)
				var offset = _add_string(string_val)
				bytecode.append(offset & 0xFF)
				bytecode.append((offset >> 8) & 0xFF)
				bytecode.append((offset >> 16) & 0xFF)
				bytecode.append((offset >> 24) & 0xFF)

			"PUSH_FLOAT":
				var f = float(parts[1])
				var i = int(f * 65536.0)
				bytecode.append(i & 0xFF)
				bytecode.append((i >> 8) & 0xFF)
				bytecode.append((i >> 16) & 0xFF)
				bytecode.append((i >> 24) & 0xFF)

			"LOAD_GLOBAL", "STORE_GLOBAL", "LOAD_LOCAL", "STORE_LOCAL":
				bytecode.append(int(parts[1]))

			"JMP", "JMP_IF_TRUE", "JMP_IF_FALSE", "CALL":
				var target = parts[1]
				var addr = labels[target]
				bytecode.append(addr & 0xFF)
				bytecode.append((addr >> 8) & 0xFF)

			_:
				if inst in _extensions:
					var subcodes = _extension_subcodes[inst]
					if not subcodes.has(parts[1]):
						push_error("Unknown sub-instruction: %s %s" % [inst, parts[1]])
						return PackedByteArray()
					var sub_code = subcodes[parts[1]]
					bytecode.append(sub_code & 0xFF)

	bytecode.append_array(_constant_pool)
	return bytecode

func _first_pass(lines: Array) -> Dictionary:
	var labels := {}
	var pc := 0

	for line in lines:
		line = line.strip_edges()
		if line == "" or line.begins_with(";"):
			continue

		if line.ends_with(":"):
			var label = line.substr(0, line.length() - 1)
			labels[label] = pc
			continue

		var parts = line.split(" ", false)
		var inst = parts[0]

		pc += 1 # opcode

		# immediates
		if inst in ["PUSH", "PUSH_FLOAT", "PUSH_STR"]:
			pc += 4
		elif inst in ["LOAD_GLOBAL", "STORE_GLOBAL", "LOAD_LOCAL", "STORE_LOCAL"]:
			pc += 1
		elif inst in ["JMP", "JMP_IF_TRUE", "JMP_IF_FALSE", "CALL"]:
			pc += 2
		elif inst in _extensions:
			pc += 1

	_constant_offset = pc

	return labels

func _add_string(s: String) -> int:
	if _string_offsets.has(s):
		return _string_offsets[s]
	
	var offset = _constant_pool.size() + _constant_offset
	var utf8_bytes = s.to_utf8_buffer()
	var str_len = utf8_bytes.size()
	_constant_pool.append(str_len & 0xFF)
	_constant_pool.append((str_len >> 8) & 0xFF)
	_constant_pool.append_array(utf8_bytes)
	_string_offsets[s] = offset
	return offset
