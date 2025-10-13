extends Node2D

var file_dialog: FileDialog

var nodes = []
var edges = []

var and_tex: Texture2D
var or_tex: Texture2D
var nand_tex: Texture2D
var nor_tex: Texture2D
var not_tex: Texture2D
var xor_tex: Texture2D

var nodes_per_dist = []

func _custom_draw_circle(node):
	var res1 = randf()
	var res2 = randf()
	var res3 = randf()
	var color = Color(res1, res2, res2)
	
	var size = 15.0
	if(node["type"] == "output"):
		size += size
	
	#draw_circle(pos, size, color, true)

func _custom_draw_line(start_id, end_id):
	var start_pos = nodes[start_id]["pos"]
	var end_pos = nodes[end_id]["pos"]

	if(nodes[start_id]["type"] == "gate"):
		start_pos = nodes[start_id]["pos"] + Vector2(and_tex.get_width(), and_tex.get_height()/2)
	if(nodes[start_id]["label"] == "NOT"):
		start_pos = nodes[start_id]["pos"] + Vector2(not_tex.get_width(), not_tex.get_height()/2)
	
	if(nodes[end_id]["type"] == "gate"):
		end_pos = nodes[end_id]["pos"] + Vector2(0, and_tex.get_height()/4)
		if(nodes[end_id]["used"]):
			end_pos = end_pos + Vector2(0, and_tex.get_height()/2)
		nodes[end_id]["used"] = true
	
	if(nodes[end_id]["label"] == "NOT"):
		end_pos = nodes[end_id]["pos"] + Vector2(0, not_tex.get_height()/2)

	var rand_x = randi_range(start_pos[0], end_pos[0])
	draw_line(start_pos, Vector2(rand_x, start_pos[1]), Color.BLACK, 5)
	draw_line(Vector2(rand_x, start_pos[1]), Vector2(rand_x, end_pos[1]), Color.BLACK, 5)
	draw_line(Vector2(rand_x, end_pos[1]), end_pos, Color.BLACK, 5)

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
		var start_node = nodes.front()
		var counter = 0;
		for node in nodes:
			if(node["dist"] != start_node["dist"]):
				start_node = node
				nodes_per_dist.push_back(counter)
				counter = 0
			counter = counter + 1
		nodes_per_dist.push_back(counter)
		
	queue_redraw()

func _draw():
	if !nodes.is_empty():
		var start_node = nodes.front();
		var screen_size = get_viewport().size
		var counter_x = 1
		var counter_y = 1
		var n = nodes_per_dist.size()
		var pos = Vector2(counter_x*screen_size.x/(n+1), counter_y*screen_size.y/(nodes_per_dist[counter_x-1]+1))
		for node in nodes:
			node["used"] = false
			if(node["dist"] != start_node["dist"]):
				start_node = node
				counter_x = counter_x + 1
				counter_y = 1
			
			pos = Vector2(counter_x*screen_size.x/(n+1), counter_y*screen_size.y/(nodes_per_dist[counter_x-1]+1))
			node["pos"] = pos
			counter_y = counter_y + 1
			
			if(node["type"] == "gate"):
				match node["label"]:
					"AND":
						draw_texture(and_tex, node["pos"])
					"OR":
						draw_texture(or_tex, node["pos"])
					"NAND":
						draw_texture(nand_tex, node["pos"])
					"NOR":
						draw_texture(nor_tex, node["pos"])
					"NOT":
						draw_texture(not_tex, node["pos"])
					"XOR":
						draw_texture(xor_tex, node["pos"])
			else:
				_custom_draw_circle(node)
				draw_string(ThemeDB.fallback_font, node["pos"], node["label"], HORIZONTAL_ALIGNMENT_LEFT, -1, 20, Color.WHITE)
		
	for edge in edges:
		_custom_draw_line(edge[0], edge[1])

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
