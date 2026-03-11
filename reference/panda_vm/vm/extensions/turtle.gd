class_name Turtle
extends Extension

'''
var width = 320
var height = 240

var img := Image.new()
var tex := ImageTexture.new()

func _ready():
    # 创建 framebuffer
    img.create(width, height, false, Image.FORMAT_RGBA8)
    img.fill(Color.black)
    
    # 初次绑定到 ImageTexture
    tex.create_from_image(img)
    
    var sprite = Sprite2D.new()
    sprite.texture = tex
    add_child(sprite)

func set_pixel(x: int, y: int, color: Color):
    img.lock()
    img.set_pixel(x, y, color)
    img.unlock()

func update_frame():
    # 只更新 ImageTexture 的内容
    tex.create_from_image(img)
'''