import sys
import json
import struct

name = sys.argv[1]

data = json.loads(open(name + ".json", "rb").read())

layers = data["layers"]

def process_cel(cel):
	if cel == 0:
		return 0xFF
	return cel - 1

for i in range(0, len(layers)):
	
	if "data" in layers[i]:
		w = layers[i]["width"]
		h = layers[i]["height"]
		layer_data = layers[i]["data"].decode("base64")
		layer_data = struct.unpack("I"*(len(layer_data)//4), layer_data)
		print "unsigned char " + name + "_layer" + str(i) + "_tilemap[" + str(w) + "*" + str(h) + "] = {"

		for y in xrange(0, h):
			print "    " + "".join([
				"0x{0:0{1}X},".format(process_cel(x), 2)
				 for x in layer_data[(y*w):((y+1)*w)]
			])
		print "};"
	elif "objects" in layers[i]:
		objects = layers[i]["objects"]
		num_objects = len(objects)
		print "int " + name + "_objects" + str(i) + "[" + str(num_objects) + "][27] = {"

		for o in range(0, len(objects)):
			obj = objects[o]
			num_edges = 4
			pos_x = obj["x"]
			pos_y = obj["y"]
			width = obj["width"]
			height = obj["height"]
			edges = [(0,0) for x in range(0, 12)]
			if "polygon" in obj:
				num_edges = len(obj["polygon"])
				for e in range(0, num_edges):
					edges[e] = (obj["polygon"][e]["x"], obj["polygon"][e]["y"])

				sum = 0

				for e in range(0, num_edges - 1):
					cur = edges[e]
					next = edges[e + 1]
					sum += (next[0] - cur[0]) * (next[1] + cur[1])
				if sum < 0:
					edges = edges[num_edges:0:-1] + edges[num_edges:]

			else:
				edges[0] = [0, 0]
				edges[1] = [0, height]
				edges[2] = [width, height]
				edges[3] = [width, 0]
				
				
			#print (pos_x, pos_y, num_edges, ",".join([("%d,%d"%(xy[0], xy[1])) for xy in edges]))
			print "    {%d,%d,%d,    %s}," % (pos_x, pos_y, num_edges, ",".join([("%d,%d"%(xy[0], xy[1])) for xy in edges]))
		print "};"