class_name Console
extends RefCounted

var _last_output: Variant = null

func output(value: Variant) -> void:
	_last_output = value
	print("Panda VM:", value)
