import sys
import math
import struct
from PIL import Image

name = sys.argv[1]
im = Image.open(name + ".bmp")

#print im.palette.palette
tile_size = int(sys.argv[2])

tiles = []

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
for y0 in xrange(0, im.size[1]//tile_size):
	for x0 in xrange(0, im.size[0]//tile_size):
		tile = []
		for y2 in xrange(0, tile_size):
			for x2 in xrange(0, tile_size):
				x = (x0)*tile_size + x2
				y = y0*tile_size + y2
				tile.append(im.getpixel((x,y)))
				
		tiles.append(tile)



print "unsigned char " + name + "_tiles["+str(len(tiles))+"]["+str((tile_size)*(tile_size))+"] = {"
tmp_y = 0
for s0 in xrange(0, len(tiles)):
	print "    {" + ",".join(["0x{0:0{1}X}".format(x, 2) for x in tiles[s0]]) + "},"


print "};"