import math
from PIL import Image

im = Image.open("background.bmp")


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

for y in xrange(0, im.size[1]):
	for x in xrange(0, im.size[0]):
		pixel = im.getpixel((x,y))
		print '0x{0:0{1}X},'.format(get_index(pixel), 2) ,
	print ""