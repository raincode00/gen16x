import math
import struct
from PIL import Image

name = "test_tileset_2"
im = Image.open(name + ".bmp")

#print im.palette.palette
sprite_size = 32

sprites = []

palette = struct.unpack("<" + "L"*(len(im.palette.palette)/4), im.palette.palette)

def process_pallete(i):
	if i != 0x00FF00FF:
		i = i | 0xFF000000
	return i
def pack_index(a, b):
	return ((a&0xF) << 4) | (b & 0xF)
	
print "unsigned int " + name + "_palette["+str(len(palette))+"] = {"
print "    " + "".join(["0x{0:0{1}X},".format(process_pallete(x), 8) for x in palette])
print "};"



"""
def get_index(rgb):
	if rgb == (214, 27, 227):
		return 0
	elif rgb == (0, 0, 0):
		return 255
	else:
		rgbf = ((rgb[0])/255.0, (rgb[1])/255.0, (rgb[2])/255.0)

		#rgbf = (math.sqrt(rgbf[0] + 1.0/255.0), math.sqrt(rgbf[1]  + 1.0/255.0), math.sqrt(rgbf[2] + 1.0/255.0))

		rgb = (int(rgbf[0]*5.99), int(rgbf[1]*5.99), int(rgbf[2]*5.99))

		idx = (rgb[0])*36 + (rgb[1])*6 + (rgb[2])
		if idx == 0:
			return 254
		return idx
"""
print "unsigned char " + name + "_tileset[256*256] = {"

for y in xrange(0, im.size[1]):
	for x in xrange(0, im.size[0]):
		pixel = im.getpixel((x,y))
		print '0x{0:0{1}X},'.format(pixel, 2) ,
	print ""


print "};"
