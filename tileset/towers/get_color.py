from PIL import Image
import os
names = [i for i in os.listdir() if ".jpg" in i and "out" not in i and "tower2" in i]
colors = {}
for name in names:
	img = Image.open(name)
	img_new = img.convert('RGBA')
	L, H = img_new.size
	color_0 = img_new.getpixel((0, 0))
	color_2 = img_new.getpixel((L/2, 50))
	id = name[6:-4]
	print("id : ",id)
	colors[id] = color_2
	for h in range(H):
		for l in range(L):
			dot = (l, h)
			color_1 = img_new.getpixel(dot)
			if color_1 == color_0:
				color_1 = color_2
				img_new.putpixel(dot, color_1)
	img_new.save("out"+name[:-3]+".png")
	

fout = open("color.txt","w")
fout.writelines(str(colors))