import bpy
import math, mathutils
import time
from datetime import datetime
import asyncore, socket
import struct
import json
import sys

# 2x3 matrix determinant
def det23(a, b, c):
	return a[0]*(b[1] - c[1]) + b[0]*(c[1] - a[1]) + c[0]*(a[1] - b[1]) 

# True if bc turns right in point of view of ab
def turnsLeft(a, b, c):
	return det23(a, b, c) > 0

def turnsRight(a, b, c):
	return not turnsLeft(a, b, c)

# Andrew's Monotone Chain Algorithm
# verts_ = [[x1, y1], [x2, y2], ...]
# Returns a counter-clockwise polygon
def convexHull(verts_):
	verts= verts_
	verts.sort()
	
	upper= [verts[0], verts[1]]
	for v in verts[2:]:
		upper.append(v)
		while len(upper) > 2 and turnsRight(*upper[-3:]):
			del upper[-2]
			
	verts.reverse()
	lower= [verts[0], verts[1]]
	for v in verts[2:]:
		lower.append(v)
		while len(lower) > 2 and turnsRight(*lower[-3:]):
			del lower[-2]
	
	del lower[0]
	del lower[-1]
	
	return upper + lower

# Converts mesh object to a convex polygon of format [[x1, y1], [x2, y2], ...]
def bmeshObjectToPolygon(ob):
	verts= ob.data.vertices
	poly= []
	for v in verts:
		v_transformed= ob.matrix_world * v.co
		poly.append(list(v_transformed[:2])) # Drop z
	poly= convexHull(poly)

	return poly

debugPrintEnabled= True
def debugPrint(*arg, **kwargs):
	if debugPrintEnabled:
		print("CloverEE:", *arg, **kwargs)

class Msg:
	name= "" # 4 characters long
	value= bytes()

class Connection(asyncore.dispatcher):
	host= "127.0.0.1"
	port= 19595
	
	connecting= False
	connected= False
	pingTimer= 0.0
	
	ReceivingState_Name= 0
	ReceivingState_ValueSize= 1
	ReceivingState_Value= 2

	receivingState= 0
	
	inMsgNameBuffer= bytes()
	inMsgValueSizeBuffer= bytes()
	inMsgValueSize= 0
	inMsgValueBuffer= bytes()

	def triggerOnMsgReceive(self):
		msg= Msg()
		msg.name= self.inMsgNameBuffer.decode("ascii")
		msg.value= self.inMsgValueBuffer
		
		self.onMsgReceive(msg) # Trigger callback
		self.inMsgNameBuffer= bytes()
		self.inMsgValueSizeBuffer= bytes()
		self.inMsgValueBuffer= bytes()
		self.receivingState= self.ReceivingState_Name

	def __init__(self, connect_callback, recv_msg_callback):
		self.connecting= True
		debugPrint("Trying to connect")
		self.dispatcher= asyncore.dispatcher.__init__(self)
		self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
		self.connect((self.host, self.port))
		self.buffer= bytes("ping", "ascii") + bytes(4)
		self.lastContactTime= datetime.now()
		self.onMsgReceive= recv_msg_callback
		self.onConnect= connect_callback
		
	def handle_connect(self):
		debugPrint("Connected")
		self.connected= True
		self.connecting= False
		self.onConnect()
		
	def handle_close(self):
		if not self.connecting:
			debugPrint("Disconnected")
		self.connected= False
		self.connecting= False
		self.close()

	def handle_read(self):
		
		if self.receivingState == self.ReceivingState_Name:
			self.inMsgNameBuffer += self.recv(4 - len(self.inMsgNameBuffer))
			if len(self.inMsgNameBuffer) == 4: # Name received
				self.receivingState= self.ReceivingState_ValueSize
				
		elif self.receivingState == self.ReceivingState_ValueSize:
			self.inMsgValueSizeBuffer += self.recv(4 - len(self.inMsgValueSizeBuffer))
			if len(self.inMsgValueSizeBuffer) == 4: # Value size received
				self.inMsgValueSize= struct.unpack("<L", self.inMsgValueSizeBuffer)[0]
				debugPrint("Value size: ", self.inMsgValueSize)
				if self.inMsgValueSize == 0:
					self.triggerOnMsgReceive()
				else:
					self.receivingState= self.ReceivingState_Value
				
			debugPrint("ValueSize recv: ", len(self.inMsgValueSizeBuffer))
			
		elif self.receivingState == self.ReceivingState_Value:
			self.inMsgValueBuffer += self.recv(self.inMsgValueSize - len(self.inMsgValueBuffer)) 
			if len(self.inMsgValueBuffer) == self.inMsgValueSize: # Value received
				self.triggerOnMsgReceive()
			
		else:
		  debugPrint("Invalid receivingState")
		

	def writable(self):
		return (len(self.buffer) > 0)

	def handle_write(self):
		debugPrint("sending")
		sent= self.send(self.buffer)
		self.buffer= self.buffer[sent:]
	
	def sendMsg(self, msg_name, value_str):
		self.buffer += bytes(msg_name, "ascii")
		debugPrint("sendMsg:", msg_name, len(bytes(msg_name, "ascii")))
		self.buffer += struct.pack("<L", len(value_str))
		self.buffer += bytes(value_str, "ascii")
	
	def update(self, dt):
		self.pingTimer += dt
		if self.pingTimer > 2.0: # Ping every two seconds
			self.buffer += bytes("ping", "ascii") + bytes(4)
			self.pingTimer= 0.0
		
	def isConnected(self):
		return self.connected
	
	def isConnecting(self):
		return self.connecting


class CloverEditorExtension:
	Mode_None= 0
	Mode_ShapeEditing= 1
	Mode_MeshEditing= 2
	
	mode= 0
	resourceRoot= ""
	cloverPath= ""
	
	layoutName= ""
	
	# Last sent/received resources as json. Set only for resources of which changes are sent
	lastResources= {}
	
	def __init__(self, send_func):
		self.sendMsg= send_func
		self.clearScene()
	
	def clearScene(self):
		
		try:
			bpy.data.scenes[0].update()
			if len(bpy.data.scenes[0].objects) > 0:
				bpy.ops.object.select_all(action="SELECT")
				bpy.ops.object.delete()
		except:
			debugPrint("Scene clearing failed", bpy.context.mode, sys.exc_info())
		
	def setLayout(self, name):
		self.layoutName= name
		bpy.context.window.screen= bpy.data.screens[name]
	
	def findSpace(self, name):
		for screen in bpy.data.screens:
			if screen.name != self.layoutName:
				continue
			for area in screen.areas:
				if area.type != name:
					continue
				for space in area.spaces:
					if space.type != name:
						continue
					return space
		return None
	
	def init2DCamera(self):
		view_space= self.findSpace("VIEW_3D")

		view_space.region_3d.view_perspective= "ORTHO"
		
		view_space.region_3d.view_matrix= \
		mathutils.Matrix((  (1.0, 0.0, 0.0, 0.0), \
		(0.0, 1.0, 0.0, 0.0), \
		(0.0, 0.0, 1.0, 0.0), \
		(0.0, 0.0, 0.0, 1.0)))

		view_space.region_3d.view_location= [0.0, 0.0, 0.0]
		
		if view_space.region_3d.view_distance < 0.1:
			view_space.region_3d.view_distance= 3.0
		
		try:
			bpy.data.scenes[0].update()
			if len(bpy.data.scenes[0].objects) > 0 and bpy.context.mode != "OBJECT":
				bpy.ops.object.mode_set(mode= "OBJECT")
		except:
			debugPrint("Object mode set failed", bpy.context.mode, sys.exc_info())

	def onResourceReceive(self, res_json):
		type_name= res_json["typeName"]
		attribs= res_json["attributes"]
		debugPrint("Resource type:", type_name)
		
		self.lastResources[type_name]= res_json
		
		if type_name == "Shape":
			self.mode= self.Mode_ShapeEditing
			self.setLayout("Shape")
			self.init2DCamera()
			self.clearScene()
			
			for base_shape in attribs["baseShapes"]:
				type= base_shape["type"]
				if type == "Polygon":
					verts= []
					center= (0.0, 0.0, 0.0)
					for json_vert in base_shape["vertices"]:
						verts += [tuple(json_vert + [0.0])]
						center= tuple([a + b for a,b in zip(center, verts[-1])])
						
					center= tuple([a/len(verts) for a in center])
					
					# Translate vertices because center is not necessarily at origin
					for i in range(len(verts)):
						verts[i]= tuple([a - c for c,a in zip(center, verts[i])])
					
					bpy.ops.object.add(
						type= "MESH",
						location= center)
					ob= bpy.context.object
					ob.name= attribs["name"]
					mesh= ob.data
					face= []
					face.extend(range(0, len(verts))) # n-gon
					mesh.from_pydata(verts, [], [face])
						
				elif type == "Circle":
					bpy.ops.object.empty_add(
						type= "CIRCLE", \
						location= tuple(base_shape["center"] + [0.0]), \
						rotation= (math.pi*0.5, 0, 0))
					ob= bpy.context.object
					ob.name= attribs["name"]
					rad= base_shape["radius"]
					ob.scale= (rad, rad, rad)
					
		elif type_name == "Texture":
			for image in bpy.data.images:
				image.user_clear() # Prevent accidentally saving multiple images in .blend
			
			self.setLayout("Model")
			# e.g.      ./data/			   ./					 ./textures/tex.png
			image_path= self.cloverPath + self.resourceRoot + res_json["path"][0] + attribs["file"]
			
			existing_image= None
			for image in bpy.data.images:
				if image.filepath == image_path:
					existing_image= image
					break
			
			image_editor= self.findSpace("IMAGE_EDITOR")
			
			try:
				if existing_image == None:
					# Create new image
					image_editor.image= bpy.data.images.load(filepath= image_path)
				else:
					# Image with same name can have different paths (e.g. "/tex/a.png", "/mat/a.png")
					existing_image.filepath= image_path
					existing_image.reload()
					image_editor.image= existing_image
			except:
				debugPrint("Error loading image:", image_path)
				
		elif type_name == "Mesh":
			self.mode= self.Mode_MeshEditing
			self.setLayout("Model")
			self.init2DCamera()
			self.clearScene()
			
			verts= []
			uvs= []
			inds= []
			
			center= (0.0, 0.0, 0.0)
			for json_vert in attribs["vertices"]:
				verts += [tuple(json_vert["position"] + [0.0])]
				center= tuple([a + b for a,b in zip(center, verts[-1])])
				uvs += [tuple(json_vert["uv"])]
			
			if len(verts) > 0:
				center= tuple([a/len(verts) for a in center])
				
				# Translate vertices because center is not necessarily at origin
				for i in range(len(verts)):
					verts[i]= tuple([a - c for c,a in zip(center, verts[i])])
				
				inds= attribs["indices"]
				
				bpy.ops.object.add(
					type= "MESH",
					location= center)
				ob= bpy.context.object
				ob.name= attribs["name"]
				mesh= ob.data
				mesh.from_pydata(verts, [], inds)
				
				bpy.ops.mesh.uv_texture_add()
				
				for tri_i in range(len(inds)):
					for ind_i in range(3):
						mesh.uv_layers[0].data[tri_i*3 + ind_i].uv= uvs[inds[tri_i][ind_i]]

		elif type_name == "Material":
			self.setLayout("Model")
			tex_identifiers= [attribs["colorMap"], attribs["normalMap"], attribs["envShadowMap"]]
			
			# Request for first found texture in material
			for id in tex_identifiers:
				if len(id) > 0:
					self.sendMsg("rsrq", json.dumps({"typeName": "Texture", "identifier": id}))
					break
		
		elif type_name == "Model":
			self.setLayout("Model")
			# Request for material and mesh
			mat_identifier= attribs["material"]
			mesh_identifier= attribs["mesh"]
			self.sendMsg("rsrq", json.dumps({"typeName": "Material", "identifier": mat_identifier}))
			self.sendMsg("rsrq", json.dumps({"typeName": "Mesh", "identifier": mesh_identifier}))
		
	def onMsgReceive(self, name, value):
		if name== "rsrc": # ReSouRCe
			self.onResourceReceive(value)
		elif name == "clpt": # CLover PaTh
			self.cloverPath= value
		elif name == "rsrt": # ReSource RooT
			self.resourceRoot= value
	
	def update(self):
		if self.mode == self.Mode_None:
			return
		
		attribs= {}
		type_name= ""
		
		# Send changes
		if self.mode == self.Mode_ShapeEditing:
			type_name= "Shape"
			attribs["name"]= self.lastResources[type_name]["attributes"]["name"]
			attribs["baseShapes"]= []
			for ob in bpy.context.visible_objects:
				shape= {}
				if ob.type == "EMPTY":
					shape["type"]= "Circle"
					shape["radius"]= abs(ob.scale[0]) # All components should be the same
					shape["center"]= [ob.location[0], ob.location[1]]
				elif ob.type == "MESH":
					shape["type"]= "Polygon"
					shape["vertices"]= bmeshObjectToPolygon(ob)
					
				attribs["baseShapes"].append(shape)
			shape_to_send= json.dumps(attribs)
		elif self.mode == self.Mode_MeshEditing and bpy.context.mode == "OBJECT": # Can't access uv in edit mode
			type_name= "Mesh"
			attribs["name"]= self.lastResources[type_name]["attributes"]["name"]
			attribs["vertices"]= []
			attribs["indices"]= []

			objects= bpy.context.visible_objects

			for ob in objects:
				if ob.type != "MESH":
					continue
				mesh= ob.data
				
				# Indices
				
				res_inds= []
				res_ind_offset= len(attribs["vertices"]) # Offset next object's indices because objects are merged in mesh resource
				
				vert_to_uv_i= {}
				for poly in mesh.polygons:
					res_tri_inds= []
					for ind in poly.loop_indices:
						vert_i= mesh.loops[ind].vertex_index
						res_tri_inds.append(vert_i + res_ind_offset)
						if len(res_tri_inds) == 3:
							res_inds.append(res_tri_inds)
							res_tri_inds= [res_tri_inds[0], res_tri_inds[2]] # Crappy triangulation
						
						if vert_i in vert_to_uv_i:
							continue
						vert_to_uv_i[vert_i]= ind
				
				attribs["indices"] += res_inds
				
				# Vertices
				for i in range(len(mesh.vertices)):
					pos= list(ob.matrix_world * mesh.vertices[i].co)
					pos= pos[:2] # Drop z
					
					if i not in vert_to_uv_i: # In case of edges without connected faces
						continue
					
					if len(mesh.uv_layers) > 0:
						uv= list(mesh.uv_layers[0].data[vert_to_uv_i[i]].uv)
					else:
						uv= [0.0, 0.0]
					attribs["vertices"].append({ "position": pos, "uv": uv })
			
			
		if len(type_name) > 0:
			res_to_send= {"typeName": type_name,
				"path": self.lastResources[type_name]["path"],
				"attributes": attribs}
			
			# Send resource only when it has changed
			if self.lastResources[type_name] != res_to_send:
				data= json.dumps(res_to_send)
				debugPrint("Sending:", data)
				self.sendMsg("rsrc", data)
				self.lastResources[type_name]= res_to_send
	
class CloverEditorExtensionModal(bpy.types.Operator):
	bl_idname= "clover.editorextension"
	bl_label= "Clover Editor Extension"

	updateInterval= 0.1
	
	extension= None

	def modal(self, context, event):
		if event.type == "ESC":
			return self.cancel(context)

		if event.type == "TIMER":
			self.update(context)

		return {'PASS_THROUGH'}

	def invoke(self, context, event):
		
		self.reconnect()
		context.window_manager.modal_handler_add(self)
		self.updateTimer= context.window_manager.event_timer_add(self.updateInterval, context.window)
		return {'RUNNING_MODAL'}

	def cancel(self, context):
		return {'CANCELLED'}

	def update(self, context):
		
		error= False
		try:
			asyncore.loop(timeout= 0.0, use_poll= True, count= 1)
		except:
			debugPrint("Unexpected error:", sys.exc_info())
			error= True
		
		if self.connection.isConnected() and not error:
			self.connection.update(self.updateInterval)
			self.extension.update()
		elif not self.connection.isConnecting() or error: # No connection and not connecting (or some unexpected error) -> try again
			self.reconnect()
			self.extension= None
	
	def reconnect(self):
		self.connection= Connection(self.onConnect, self.onMsgReceive)

	def onConnect(self):
		self.extension= CloverEditorExtension(self.connection.sendMsg)

	def onMsgReceive(self, msg):
		debugPrint("Msg received:", msg.name)
		debugPrint("Value:", msg.value)
		if msg.name == "pong":
			return
		self.extension.onMsgReceive(msg.name, json.loads(msg.value.decode("ascii")))
		
class AddCircleShapeModal(bpy.types.Operator):
	bl_idname= "clover.addcircleshape"
	bl_label= "Clover Add CircleShape"

	def invoke(self, context, event):
		bpy.ops.object.empty_add(type= "CIRCLE", rotation= (math.pi*0.5, 0, 0))
		bpy.context.object.scale= (0.05, 0.05, 0.05)
		return {"FINISHED"}

	def cancel(self, context):
		return {'CANCELLED'}
			
class AddPolygonShapeModal(bpy.types.Operator):
	bl_idname= "clover.addpolygonshape"
	bl_label= "Clover Add PolygonShape"

	def invoke(self, context, event):
		bpy.ops.mesh.primitive_plane_add()
		bpy.context.object.scale= (0.05, 0.05, 0.05)
		return {"FINISHED"}

	def cancel(self, context):
		return {'CANCELLED'}
			
def register():
	bpy.utils.register_class(CloverEditorExtensionModal)
	bpy.utils.register_class(AddCircleShapeModal)
	bpy.utils.register_class(AddPolygonShapeModal)
	bpy.ops.clover.editorextension("INVOKE_DEFAULT") # Start when Blender starts
	
def unregister():
	bpy.utils.unregister_class(CloverEditorExtensionModal)
	bpy.utils.unregister_class(AddCircleShapeModal)
	bpy.utils.unregister_class(AddPolygonShapeModal)
	
register();