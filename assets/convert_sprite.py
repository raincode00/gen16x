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
					for x2 in xrange(0, 4):
						x = (x0)*sprite_size + (x1)*8 + (2*x2)
						y = y0*sprite_size + y1*8 + y2
						sprite_tile.append(pack_index(im.getpixel((x,y)),  im.getpixel((x+1,y))))
						
				sprite_tiles.append(sprite_tile)
		sprites.append(sprite_tiles)

im2 = Image.new('RGB', (sprite_size, len(sprites)*sprite_size), "black")
pixels = im2.load() 


print "unsigned char " + name + "_tiles["+str(len(sprites))+"]["+str((sprite_size/8)*(sprite_size/8))+"][32] = {"
tmp_y = 0
for s0 in xrange(0, len(sprites)):
	print "    {"
	for s1 in xrange(0, len(sprites[s0])):
		#print len(sprites[s0][s1])

		print "        " + "".join(["0x{0:0{1}X},".format(x, 2) for x in sprites[s0][s1]])

		for s2 in xrange(0, len(sprites[s0][s1])):
			subx = s1 % (sprite_size//8)
			suby = s1 // (sprite_size//8)
			subx0 = ((s2*2) % 8)
			suby0 = ((s2*2) // 8)
			x = subx*8 + subx0
			y = s0*sprite_size + suby*8 + suby0
			index = sprites[s0][s1][s2]
			color1 = (index&0x0F)
			color0 = (index&0xF0) >> 4
			#print s0, s1, s2,":", subx0, suby0, ":", x, y
			pixels[x, y] = (ord(im.palette.palette[color0*4 + 2]),ord(im.palette.palette[color0*4 + 1]),ord(im.palette.palette[color0*4]))
			pixels[x+1, y] = (ord(im.palette.palette[color1*4 + 2]),ord(im.palette.palette[color1*4 + 1]),ord(im.palette.palette[color1*4]))
	#tmp_y += sprite_size


		#print '0x{0:0{1}X},'.format(pixel, 2),
	print "    },"	
print "};"

im2.save("debug.bmp")