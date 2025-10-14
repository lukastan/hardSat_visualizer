extends Sprite2D

func _ready():
	var viewport_size = get_viewport_rect().size
	var tex_size = texture.get_size()

	position = viewport_size / 2
	scale = viewport_size / tex_size
