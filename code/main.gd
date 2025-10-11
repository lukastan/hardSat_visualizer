extends Node2D

var file_dialog: FileDialog

var nodes = {}
var edges = []

func _on_file_selected(path: String):
	print("Selected file: ", path)
	
	var file = FileAccess.open(path, FileAccess.READ)
	if not file:
		print("Not a file!")
		return
	
	var json_string = file.get_as_text()
	file.close()
	
	var result = JSON.new()
	var error = result.parse(json_string)
	if error != OK:
		print("Error in parse: ", result.error_string())
		return
		
	var data = result.data
	print("Our data: ", data)
	
	if data.has("edges"):
		edges = data["edges"]
	
	if data.has("nodes"):
		nodes = data["nodes"];
		
	queue_redraw()

func _draw():
	for edge in edges:
		var fir = edge[0]
		var sec = edge[1]
		draw_line(Vector2(1, 1), Vector2(1, 1), Color.WHITE, 5)
	
	for id in nodes:
		draw_circle(Vector2(0, 0), 15, Color.SKY_BLUE)
		draw_string(ThemeDB.fallback_font, Vector2(20, 5), id.label, HORIZONTAL_ALIGNMENT_LEFT, -1, 16, Color.WHITE)

func _ready():
	file_dialog = FileDialog.new()
	file_dialog.access = FileDialog.ACCESS_FILESYSTEM
	file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	file_dialog.filters = ["*.json"]
	add_child(file_dialog)
	
	file_dialog.file_selected.connect(_on_file_selected)
	
	file_dialog.popup_centered()
