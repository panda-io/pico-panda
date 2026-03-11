class_name Handler
extends RefCounted

var _suspended_task: Task = null
var _bytecode: PackedByteArray
var _entry_addr: int = 0