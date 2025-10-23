extends Control

func _ready():
	size = get_viewport().get_visible_rect().size
	get_viewport().connect("size_changed", Callable(self, "_on_viewport_resized"))

func _on_viewport_resized():
	size = get_viewport().get_visible_rect().size
