import json
import struct
name = "test_map"

data = json.loads(open(name + ".json", "rb").read())

layers = data["layers"]

def process_cel(cel):
	if cel == 0:
		return 0xFF
	return cel - 1

for i in range(0, len(layers)):
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