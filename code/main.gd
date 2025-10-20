extends Node2D

@onready var font : Font = preload("res://fonts/times.ttf")

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

func _custom_draw_line(start_node, end_node):
	var start_pos = start_node["pos"]
	var end_pos = end_node["pos"]

	if(start_node["type"] == "gate"):
		start_pos = start_node["pos"] + Vector2(and_tex.get_width(), and_tex.get_height() * 0.5)
	if(start_node["label"] == "NOT"):
		start_pos = start_node["pos"] + Vector2(not_tex.get_width(), not_tex.get_height() * 0.5)
	
	if(end_node["type"] == "gate"):
		end_pos = end_node["pos"] + Vector2(5, and_tex.get_height() * 0.25)
		if(end_node["used"]):
			end_pos = end_pos + Vector2(0, and_tex.get_height() * 0.5)
		end_node["used"] = true
	
	if(end_node["label"] == "NOT"):
		end_pos = end_node["pos"] + Vector2(0, not_tex.get_height() * 0.5)

	var dif = (end_pos[0]-start_pos[0])/5
	var rand_x = randi_range(start_pos[0] + dif, end_pos[0] - dif)
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
		var indices = range(data["nodes"].size())
		indices.sort_custom(func(a, b):
			return data["nodes"][a].get("dist", -1) > data["nodes"][b].get("dist", -1)
		)
		for i in indices:
			nodes.append(data["nodes"][i])
			
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
			node["pos"] = pos * 1.0
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
				var ofset = Vector2(0, 5)
				if(node["type"] == "input"):
					ofset -= Vector2(15, 0)
				else:
					ofset += Vector2(10, 0)
				draw_string(font, node["pos"] + ofset, node["label"], HORIZONTAL_ALIGNMENT_LEFT, -1, 20, Color.BLACK)
		
	var nodes_by_id = {}
	for node in nodes:
		nodes_by_id[node["id"]] = node	
	
	for edge in edges:
		print(edge)
		_custom_draw_line(nodes_by_id[edge[0]], nodes_by_id[edge[1]])

func _ready():
	var png_path = "res://png/"
	and_tex = load(png_path + "And.png")
	or_tex = load(png_path + "Or.png")
	nand_tex = load(png_path + "Nand.png")
	nor_tex = load(png_path + "Nor.png")
	not_tex = load(png_path + "Not.png")
	xor_tex = load(png_path + "Xor.png")
	
	file_dialog = FileDialog.new()
	file_dialog.access = FileDialog.ACCESS_FILESYSTEM
	file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	file_dialog.filters = ["*.json"]
	add_child(file_dialog)
	
	file_dialog.file_selected.connect(_on_file_selected)
	
	file_dialog.popup_centered()
