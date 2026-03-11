extends Node2D


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	print("start vm")
	var _vm: VM = VM.new()
	var _assembler: Assembler = Assembler.new()
	var bytecode = _assembler.assemble(
	"""
        PUSH 42
        PUSH 100
	""")
	var task = _vm.add_task(bytecode)
	print(bytecode.size())
	_vm.tick()
	print("tick finished")


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass

'''
; comment
start:
    PUSH 10
    INT_TO_FIXED
    CALL foo
    JMP end

foo:
    LOAD_LOCAL 0
    ADD_INT
    RET

end:
    TASK TASK_EXIT
'''
