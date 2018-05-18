import sys
import math
import struct
from PIL import Image

name = sys.argv[1]
im = Image.open(name + ".bmp")

#print im.palette.palette
sprite_size = int(sys.argv[2])

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
for y0 in xrange(0, im.size[1]//sprite_size):
	for x0 in xrange(0, im.size[0]//sprite_size):
		sprite_tiles = []
		for y1 in xrange(0, sprite_size/8):
			for x1 in xrange(0, sprite_size/8):
				sprite_tile = []
				for y2 in xrange(0, 8):
					for x2 in xrange(0, 8):
						x = (x0)*sprite_size + (x1)*8 + (x2)
						y = y0*sprite_size + y1*8 + y2
						sprite_tile.append(im.getpixel((x,y)))
						
				sprite_tiles.append(sprite_tile)
		sprites.append(sprite_tiles)



print "unsigned char " + name + "_tiles["+str(len(sprites))+"]["+str((sprite_size/8)*(sprite_size/8))+"][32] = {"
tmp_y = 0
for s0 in xrange(0, len(sprites)):
	print "    {"
	for s1 in xrange(0, len(sprites[s0])):
		#print len(sprites[s0][s1])

		print "        " + "".join(["0x{0:0{1}X},".format(x, 2) for x in sprites[s0][s1]])

		
	print "    },"	
print "};"