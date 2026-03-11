class_name Event
extends RefCounted

const EVENT_START := 0x01
const EVENT_TICK := 0x02

var _event_id: int = 0
var _regs: PackedInt32Array = PackedInt32Array()

func _init():
    _regs.resize(Task.REGISTER_COUNT)

func set_data(event_id: int, regs: PackedInt32Array):
    _event_id = event_id
    for i in range(regs.size(), Task.REGISTER_COUNT):
        _regs[i] = regs[i]