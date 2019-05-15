from PIL import Image
img = Image.open('HeavyArcher0.png')
img_new = img.convert('RGBA')
L, H = img_new.size
color_0 = img_new.getpixel((0, 0))

for h in range(H):
    for l in range(L):
        dot = (l, h)
        color_1 = img_new.getpixel(dot)
        if color_1 == color_0:
            color_1 = (255, 0, 0, 255)
            img_new.putpixel(dot, color_1)

img_new.save('test.png')
