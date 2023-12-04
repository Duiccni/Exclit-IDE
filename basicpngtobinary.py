import matplotlib.image as mpimg
image_path = "E:\\OLD_I_AKA_PROJECTS\\Python\\New\\font1.png"
image = mpimg.imread(image_path)

image_size = image.shape

bytes = [image_size[1] & 0b11111111, (image_size[1] >> 8) & 0b11111111, (image_size[1] >> 16) & 0b11111111, (image_size[1] >> 24) & 0b11111111,
         image_size[0] & 0b11111111, (image_size[0] >> 8) & 0b11111111, (image_size[0] >> 16) & 0b11111111, (image_size[0] >> 24) & 0b11111111]

for y in range(image_size[0] - 1, -1, -1):
    for x in range(image_size[1]):
        bytes.append(int(image[y][x][2] * 255))
        bytes.append(int(image[y][x][1] * 255))
        bytes.append(int(image[y][x][0] * 255))

file = open("image.bin", "wb")

bytess = bytearray(bytes)
file.write(bytess)

file.close()
