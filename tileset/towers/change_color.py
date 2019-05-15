from PIL import Image
import os
import numpy as np

def fit_color(color_0,color_1):
	int_color_0 = np.array([int(i) for i in color_0])
	
	int_color_1 = np.array([int(i) for i in color_1])
	if (sum(abs(int_color_0-int_color_1))<100):
		return True
	else:
		return False

names = [i for i in os.listdir() if ".jpg" in i and "out" not in i]
old_colors = {'-1': (207, 25, 206, 255), '0': (57, 57, 57, 255), '1': (93, 176, 84, 255), '2': (79, 209, 185, 255), '3': (252, 148, 135, 255)}
new_colors = {'-1': (207, 25, 206, 255), '0': (57, 57, 57, 255), '1': (93, 206, 84, 255), '2': (39, 79, 185, 255), '3': (252, 148, 135, 255)}
for name in names:
	img = Image.open(name)
	img_new = img.convert('RGBA')
	L, H = img_new.size
	id = name[6:-4]
	print("id:",id)
	if(id != "1" and id != "2"):
		continue
	color_0 = old_colors[id]
	color_2 = new_colors[id]
	for h in range(H):
		for l in range(L):
			dot = (l, h)
			color_1 = img_new.getpixel(dot)
			if (fit_color(color_0, color_1)):
				color_1 = color_2
				img_new.putpixel(dot, color_1)

	img_new.save("G:/prog/FC16/UI/Qt/FCUI/FC16UIResource/tileset/towers/"+name[:-4]+".png")
