extends Node2D

var file_dialog: FileDialog

var nodes = []
var edges = []
var node_pos = {}
var and_tex: Texture2D
var or_tex: Texture2D
var nand_tex: Texture2D
var nor_tex: Texture2D
var not_tex: Texture2D
var xor_tex: Texture2D

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
	# print("Our data: ", data)
	
	if data.has("edges"):
		edges = data["edges"]
	
	if data.has("nodes"):
		nodes = data["nodes"]
		nodes.sort_custom(func(a, b):
			return a.get("dist", -1) > b.get("dist", -1)
		)
		
	queue_redraw()

func _draw():
	if !nodes.is_empty():
		var start_node = nodes.front();
		var vertical = 100
		var horizontal = 100
		var pos = Vector2(100, 100)
		for node in nodes:
			if(node["dist"] != start_node["dist"]):
				start_node = node
				vertical = 100
				horizontal += 100
				
			pos = Vector2(horizontal, vertical)
			if(node["type"] == "gate"):
				match node["label"]:
					"AND":
						draw_texture(and_tex, pos)
					"OR":
						draw_texture(or_tex, pos)
					"NAND":
						draw_texture(nand_tex, pos)
					"NOR":
						draw_texture(nor_tex, pos)
					"NOT":
						draw_texture(not_tex, pos)
					"XOR":
						draw_texture(xor_tex, pos)
			else:
				draw_string(ThemeDB.fallback_font, pos, node["label"], HORIZONTAL_ALIGNMENT_LEFT, -1, 20, Color.WHITE)
			node_pos[node.id] = pos
			vertical += 100
		
	for edge in edges:
		var start_pos = node_pos[edge[0]]
		var end_pos = node_pos[edge[1]]
		draw_line(start_pos, end_pos, Color.BLACK, 5)

func _ready():
	and_tex = load("res://PNG/And.png")
	or_tex = load("res://PNG/Or.png")
	nand_tex = load("res://PNG/Nand.png")
	nor_tex = load("res://PNG/Nor.png")
	not_tex = load("res://PNG/Not.png")
	xor_tex = load("res://PNG/Xor.png")
	
	file_dialog = FileDialog.new()
	file_dialog.access = FileDialog.ACCESS_FILESYSTEM
	file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	file_dialog.filters = ["*.json"]
	add_child(file_dialog)
	
	file_dialog.file_selected.connect(_on_file_selected)
	
	file_dialog.popup_centered()
