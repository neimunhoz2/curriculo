extends Sprite

var cor=Color("#d5020c")
var oldpos
var savescenes=false
var tabelagaussiana=[[0,0,0],[0,0,0],[0,0,0]]
var FILTLARG=3
var cini=1

func create_polygon_from_sprite(nodeparent,dir,index,maxidx):
	var pngpath="l_"+str("%04d" % (maxidx-index))+"_"+str(index)
	var texturebutton=TextureButton.new()
	texturebutton.name="bt"+str(index)
	var sprite=Sprite.new()
	sprite.texture=load(dir+pngpath+".png")
	sprite.centered=false
	var texture = sprite.texture
	var texture_size = sprite.texture.get_size()
	var image = texture.get_data()
	var bitmap = BitMap.new()
	bitmap.create_from_image_alpha(image, 0.5) # 0.1 (default threshold).0.5?
	
	var bitmap_rect = Rect2(Vector2(0, 0), bitmap.get_size())
	# Grow the bitmap if you need (we don't need it in this case).
	bitmap.grow_mask(2, bitmap_rect) # 2
	#bitmap.position = sprite.position - (texture_size / 2)
	texturebutton.texture_click_mask=bitmap
	var polygons = bitmap.opaque_to_polygons(bitmap_rect, 0) # 2 (default epsilon).
	if polygons.size() > 0:
		for i in range(polygons.size()):
			var polygon = Polygon2D.new()
			polygon.polygon = polygons[i]
			polygon.texture = texture
			if sprite.centered:
				polygon.position = sprite.position - (texture_size / 2)
			else:
				polygon.position = sprite.position
			#polygon.scale = sprite.scale
			#polygon.position *= polygon.scale
			polygon.name = "Sprite"
			texturebutton.add_child(polygon)
			
		texturebutton.connect("pressed", self, "_on_button_down",[index-1])		
		nodeparent.add_child(texturebutton)
		texturebutton.owner=nodeparent
		texturebutton.get_child(0).owner=nodeparent

func _ready():
	var gama=1.0
	var x
	var y
	var w=.340
	var soma=0
	var i
	var j
	i=0
	y=-cini
	while i<FILTLARG:
		j=0
		x=-cini
		while j<FILTLARG:
			tabelagaussiana[i][j]=(cos(x*w)*cos(x*w)*exp(-(x*x+y*y)/(2.0*gama*gama))*cos(y*w)*cos(y*w))*1000
			soma+=tabelagaussiana[i][j]
			j+=1
			x+=1
		i+=1
		y+=1
	i=0
	while i<FILTLARG:
		j=0
		while j<FILTLARG:
			tabelagaussiana[i][j]/=soma
			j+=1
		i+=1
	var imgpath=get_parent().get_parent().imgpath
	var quantimg=get_parent().get_parent().quantimg
	
	if savescenes:
		for k in range(1,quantimg+1):
			create_polygon_from_sprite(get_node(imgpath),"res://sprites/jogos/pintar/"+imgpath+"/",k,quantimg)
		var packed_scene = PackedScene.new()
		packed_scene.pack(get_node(imgpath))
		ResourceSaver.save("res://jogos/pintar/"+imgpath+".tscn", packed_scene)
	else:
		var par=get_node("vpctn/pintura").get_node(imgpath)
		i=0
		var child
		while i < par.get_child_count ( ):
			child=par.get_child(i)
			if child!=null:
				child.connect("pressed", self, "_on_button_down",[i])
			i+=1
	
 
func _input(event):
	return
	if event is InputEventMouseMotion:
		if event.button_mask>0:
			$pontapincel.position=get_global_mouse_position()-get_position()
	if event is InputEventMouseButton:
		if not event.pressed:	
			$pontapincel.position=oldpos
		else:			
			oldpos=$pontapincel.position
			$pontapincel.position=get_global_mouse_position()-get_position()

func _on_btred_pressed():
	get_node("/root/home").tocamusicaouefeito("selectsound",true,0)
	cor=Color("#d5020c")
	$pontapincel.texture=load("res://sprites/jogos/pintar/pincelred.png")
	$pontapincel.position=get_global_mouse_position()-get_position()
	oldpos=$pontapincel.position

func _on_btyellow_pressed():
	get_node("/root/home").tocamusicaouefeito("selectsound",true,0)
	cor=Color("#ffe900")
	$pontapincel.texture=load("res://sprites/jogos/pintar/pincelyellow.png")
	$pontapincel.position=get_global_mouse_position()-get_position()
	oldpos=$pontapincel.position

func _on_btgreen_pressed():
	get_node("/root/home").tocamusicaouefeito("selectsound",true,0)
	cor=Color("#b5d001")
	$pontapincel.texture=load("res://sprites/jogos/pintar/pincelgreen.png")
	$pontapincel.position=get_global_mouse_position()-get_position()
	oldpos=$pontapincel.position


func _on_btpurple_pressed():
	get_node("/root/home").tocamusicaouefeito("selectsound",true,0)
	cor=Color("#99215f")
	$pontapincel.texture=load("res://sprites/jogos/pintar/pincelpurple.png")
	$pontapincel.position=get_global_mouse_position()-get_position()
	oldpos=$pontapincel.position


func _on_btblue_pressed():
	get_node("/root/home").tocamusicaouefeito("selectsound",true,0)
	cor=Color("#367aef")
	$pontapincel.texture=load("res://sprites/jogos/pintar/pincelblue.png")
	$pontapincel.position=get_global_mouse_position()-get_position()
	oldpos=$pontapincel.position


func _on_btwhite_pressed():
	get_node("/root/home").tocamusicaouefeito("selectsound",true,0)
	cor=Color.white
	$pontapincel.texture=load("res://sprites/jogos/pintar/pincellwhite.png")
	$pontapincel.position=get_global_mouse_position()-get_position()
	oldpos=$pontapincel.position


func _on_btblack_pressed():
	get_node("/root/home").tocamusicaouefeito("selectsound",true,0)
	cor=Color("#010101")
	$pontapincel.texture=load("res://sprites/jogos/pintar/pincelblack.png")
	$pontapincel.position=get_global_mouse_position()-get_position()
	oldpos=$pontapincel.position


func _on_btdarkblue_pressed():
	get_node("/root/home").tocamusicaouefeito("selectsound",true,0)
	cor=Color("#0c1c5d")
	$pontapincel.texture=load("res://sprites/jogos/pintar/pinceldarkblue.png")
	$pontapincel.position=get_global_mouse_position()-get_position()
	oldpos=$pontapincel.position


func _on_btdarkgreen_pressed():
	get_node("/root/home").tocamusicaouefeito("selectsound",true,0)
	cor=Color("#334914")
	$pontapincel.texture=load("res://sprites/jogos/pintar/pinceldarkgreen.png")
	$pontapincel.position=get_global_mouse_position()-get_position()
	oldpos=$pontapincel.position


func _on_btdarkgray_pressed():
	get_node("/root/home").tocamusicaouefeito("selectsound",true,0)
	cor=Color("#525252")
	$pontapincel.texture=load("res://sprites/jogos/pintar/pincelgray.png")
	$pontapincel.position=get_global_mouse_position()-get_position()
	oldpos=$pontapincel.position


func _on_btorange_pressed():
	get_node("/root/home").tocamusicaouefeito("selectsound",true,0)
	cor=Color("#ff5300")
	$pontapincel.texture=load("res://sprites/jogos/pintar/pincelorange.png")
	$pontapincel.position=get_global_mouse_position()-get_position()
	oldpos=$pontapincel.position


func _on_btviolet_pressed():
	get_node("/root/home").tocamusicaouefeito("selectsound",true,0)
	cor=Color("#eb3c9b")
	$pontapincel.texture=load("res://sprites/jogos/pintar/pincelviolet.png")
	$pontapincel.position=get_global_mouse_position()-get_position()
	oldpos=$pontapincel.position


func _on_btfinalizar_pressed():
	get_node("/root/home").tocamusicaouefeito("childrenyeah",true,2)
	get_parent().get_parent().hide()
	get_node("parabens").popup_centered()
	get_node("timerparabens").start()


func _on_timerparabens_timeout():
	get_node("parabens").hide()
	get_parent().get_parent().get_node("menulateral")._on_back_pressed()


func _on_TextureButton_pressed():
	get_node("timerparabens").stop()
	get_node("parabens").hide()


func _on_parabens_popup_hide():
	get_node("/root/home").tocamusicaouefeito("childrenyeah",false,2)
	get_parent().get_parent().get_node("menulateral")._on_back_pressed()


func _on_button_down(i):
	get_node("/root/home").tocamusicaouefeito("pintando",true,0)
	var par=get_node("vpctn/pintura").get_node(get_parent().get_parent().imgpath)
	var path=par.get_child((i))
	print(path)
	if path != null:
		path.get_child(0).color=cor
		
var thread=null
func check_validity(x,y,width,height):
	if x < 0 or x >= width or y < 0 or y >= height:
		return false
	return true
	
func blend( color1,  color2,  alpha):
	var rb = color1 & 0xff00ff;
	var g  = color1 & 0x00ff00;
	rb += ((color2 & 0xff00ff) - rb) * alpha >> 8;
	g  += ((color2 & 0x00ff00) -  g) * alpha >> 8;
	return (rb & 0xff00ff) | (g & 0xff00);
	
func salvapintura(userdata):
	get_tree().get_root().set_disable_input(true)
	var pol2D
	var image
	var w
	var h
	var corf
	var cori
	var corif
	var alpha
	var imgfinal=Image.new()
	var curpint=get_node("vpctn/pintura").get_node(get_parent().get_parent().imgpath)
	print(OS.get_user_data_dir())
	pol2D=curpint.get_child(0).get_child(0)
	image =pol2D.get_texture().get_data()
	w=image.get_width()
	h=image.get_height()
	imgfinal.create (w,h, false, Image.FORMAT_RGBA8 )
	imgfinal.lock()
	var x
	var y
	var cortemp
	var blended_color
	var count=curpint.get_child_count ( )
	$load.visible=true
	$load.play()
	"""
	for i in range(0,count):
		pol2D=curpint.get_child(i).get_child(0)
		cori=pol2D.color
		image =pol2D.get_texture().get_data()
		image.lock()
		x=0			
		while x < w:
			y=0
			while y < h:				
				corf=image.get_pixel(x,y)
				corif=imgfinal.get_pixel(x,y)
				cortemp=Color((corf.r*cori.r),(corf.g*cori.g),(corf.b*cori.b),corf.a)
				blended_color = corif.blend(cortemp)
				if blended_color.a!=0.0:
				#if corf.a!=0.0:
					#imgfinal.set_pixel(x,y,Color((corf.r*cori.r*corf.a+corif.r*corif.a*(1.0-corf.a))/alpha,(corf.g*cori.g*corf.a+corif.g*corif.a*(1.0-corf.a))/alpha,(corf.b*cori.b*corf.a+corif.b*corif.a*(1.0-corf.a))/alpha,alpha))
					
					imgfinal.set_pixel(x,y,blended_color)
					#imgfinal.set_pixel(x,y,Color((corf.r*cori.r),(corf.g*cori.g),(corf.b*cori.b),corf.a))
				y+=1
			x+=1
		image.unlock()
		"""
	"""
	var cf
	var ra
	var ga
	var ba
	var m	
	var ya
	var xa
	var l
	x=0		
	while x < w:
		y=0
		while y < h:				
			ra=0
			ga=0
			ba=0
			m=0
			ya=y-cini
			while m<FILTLARG:
				l=0
				xa=x-cini
				while l<FILTLARG:
					if check_validity(xa, ya,w,h):
						cf=imgfinal.get_pixel(xa,ya);
						ra+=tabelagaussiana[m][l]*cf.r
						ga+=tabelagaussiana[m][l]*cf.g
						ba+=tabelagaussiana[m][l]*cf.b
					l+=1
					xa+=1			
				m+=1
				ya+=1
			cf=Color(ra,ga,ba)
			imgfinal.set_pixel(x,y,cf)
			y+=1
		x+=1
	"""
	imgfinal.unlock()
	#imgfinal.save_png(OS.get_user_data_dir()+"/pintura.png")
	var vpimg=$vpctn/pintura.get_texture().get_data()
	vpimg.flip_y()
	vpimg.save_png(OS.get_user_data_dir()+"/pintura.png")
	
	$load.stop()
	$load.visible=false
	$loaded.visible=true
	$tmerloaded.start()
	#print(Engine.get_method_list ( ))
	if Engine.has_singleton("GodotShare"):
		var GodotShare = Engine.get_singleton("GodotShare")
		GodotShare.sharePic(OS.get_user_data_dir()+"/pintura.png", "title", "subject", "text")
	get_tree().get_root().set_disable_input(false)

func _exit_tree():
	if thread!=null:
		thread.wait_to_finish()
	
func _on_btsave_pressed():
	thread = Thread.new()
	thread.start(self, "salvapintura", "Wafflecopter")

func _on_tmerloaded_timeout():
	$loaded.visible=false


func _on_FileDialog_confirmed():
	thread = Thread.new()
	thread.start(self, "salvapintura", "Wafflecopter")


func _on_Node2D_tree_exiting():
	pass # Replace with function body.
