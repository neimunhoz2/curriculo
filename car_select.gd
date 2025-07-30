extends Node2D

var town = null
var massa=194.0
var carga=130.0
var xpos=0
var zpos=0
var altitude=310
func _process(_delta):
	if Input.is_action_just_pressed("back"):
		_on_Back_pressed()

var tt=null
func _load_scene(car):
	var caro=load(car)	
	tt = caro.instance()
	tt.get_child(0).mass=massa+carga
	tt.set_translation (Vector3 (-xpos,altitude,-zpos))
	tt.set_name("car")
	print(tt.get_translation())
	town = load("res://town_scene.tscn").instance()
	
	town.get_node("InstancePos").add_child(tt)
	town.get_node("Back").connect("pressed", self, "_on_Back_pressed")
	get_parent().add_child(town)
	hide()


func _on_Back_pressed():
	if is_instance_valid(town):
		# Currently in the town, go back to main menu.
		town.queue_free()
		show()
	else:
		# In main menu, exit the game.
		get_tree().quit()


func _on_MiniVan_pressed():
	_load_scene("res://car_base.tscn")

	
const imgW=1623.0
const imgH=700.0
const terrainW=4320.9
const terrainH=1683.23
	
var CAVE_LIMIT =imgW*0.5
var CAVE_LIMITY = imgH*0.5
onready var cave = $mapa

func _unhandled_input(event):
	if event is InputEventMouseMotion and event.button_mask > 0:
		var rel_x = event.relative.x
		var rel_y = event.relative.y
		var cavepos = cave.position
		var alvo=get_node("alvo")
		var alvopos = alvo.position		
		cavepos.x += rel_x
		cavepos.y += rel_y
		alvopos.x += rel_x
		alvopos.y += rel_y
		if cavepos.x < -CAVE_LIMIT:
			cavepos.x = -CAVE_LIMIT
		elif cavepos.x > 0:
			cavepos.x = 0
		if cavepos.y < -CAVE_LIMITY:
			cavepos.y = -CAVE_LIMITY
		elif cavepos.y > 0:
			cavepos.y = 0
		cave.position = cavepos
		alvo.position=alvopos
	elif event is InputEventMouseButton and event.button_mask > 0:
		var pos=(event.position-cave.position)
		xpos=-((pos.x-(imgW*0.5)))*terrainW/imgW
		zpos=-((pos.y-(imgH*0.5)))*terrainH/imgH
		var alvo=get_node("alvo")
		alvo.position=event.position
		#alvo.set_offset(event.position)
		print(pos)



func _on_TextEdit_text_changed():
	altitude=get_node("Panel2").get_node("Panel").get_node("txtAltitude").text
	print(altitude)


func _on_Button_down():	
	get_node("Panel2").visible=!get_node("Panel2").visible 
	if get_node("Panel2").visible:
		get_node("Button").text="-"
	else:
		get_node("Button").text="+"
	


func _on_txtMassa_text_changed():
	massa=float(get_node("Panel2").get_node("Panel").get_node("txtMassa").text)


func _on_txtCarga_text_changed():
	carga=float(get_node("Panel2").get_node("Panel").get_node("txtCarga").text)



