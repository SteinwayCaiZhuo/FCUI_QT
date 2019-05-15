from PIL import Image
import os
names = [i for i in os.listdir() if ".png" in i and "out" not in i]
colors = {'0': (0, 0, 0, 88), '1': (30, 194, 40, 128), '2': (0, 114, 216, 128), '3': (255, 53, 49, 118)}
for name in names:
	img = Image.open(name)
	img_new = img.convert('RGBA')
	L, H = img_new.size
	color_0 = img_new.getpixel((0, 0))
	id = name[-5]
	color_2 = colors[id]
	for h in range(H):
		for l in range(L):
			dot = (l, h)
			color_1 = img_new.getpixel(dot)
			if color_1 == color_0:
				color_1 = color_2
				img_new.putpixel(dot, color_1)

	img_new.save("G:/prog/FC16/UI/Qt/FCUI/FC16UIResource/tileset/workspace/"+name)
