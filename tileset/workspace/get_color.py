from PIL import Image
import os
names = [i for i in os.listdir() if ".png" in i and "out" not in i and "LightArcher" in i]
colors = {}
for name in names:
	img = Image.open(name)
	img_new = img.convert('RGBA')
	L, H = img_new.size
	color_0 = img_new.getpixel((0, 0))
	color_2 = img_new.getpixel((L/2, H/2))
	id = name[-5]
	colors[id] = color_2
	for h in range(H):
		for l in range(L):
			dot = (l, h)
			color_1 = img_new.getpixel(dot)
			if color_1 == color_0:
				color_1 = color_2
				img_new.putpixel(dot, color_1)

	//img_new.save("G:/prog/FC16/UI/Qt/FCUI/FC16UIResource/tileset/workspace/"+name)

fout = open("color.txt","w")
fout.writelines(str(colors))