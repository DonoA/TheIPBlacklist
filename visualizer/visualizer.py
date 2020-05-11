from PIL import Image, ImageDraw, ImageFont
import sys, os
import re

IP_regex = r"(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})(\/(\d+))?"

RES = 256
SCALE = 4
IMAGE_RES = SCALE*RES + 17
GRID_SIZE = (SCALE*RES/16) + 1
FONT_SIZE = 10
MAX_COUNT = 2**16

def newImage():
    return Image.new('RGB', (IMAGE_RES, IMAGE_RES), color=(255, 255, 255))

def drawPixels(img, out_pixels):
    total_count = 0
    draw = ImageDraw.Draw(img)

    for b1 in range(256):
        box_start = (1 + int(b1 / 16) * GRID_SIZE, 1 + (b1 % 16) * GRID_SIZE)
        for b2 in range(256):
            if out_pixels[b1][b2] == 0:
                continue

            point_start = (int(b2 / 16) * SCALE, (b2 % 16) * SCALE)
            abs_start = (box_start[0] + point_start[0], box_start[1] + point_start[1])

            count = out_pixels[b1][b2]
            percent = min(count / MAX_COUNT, 1.0)

            greyvalue = 255 - (55 + int(percent * 200))
            
            draw.rectangle((abs_start[0], abs_start[1], abs_start[0] + SCALE - 1, abs_start[1] + SCALE - 1), fill=(greyvalue, greyvalue, greyvalue))
            total_count += min(count, MAX_COUNT)
    
    return total_count

def drawOverlay(img):
    draw = ImageDraw.Draw(img)
    font = ImageFont.truetype("Roboto-Regular.ttf", FONT_SIZE)

    for x in range(16):
        for y in range(16):
            draw.text((2 + x * GRID_SIZE, 1 + y * GRID_SIZE), str(x * 16 + y), font=font, fill=(255, 0, 0))

    for y in range(17):
        draw.line((0, y*GRID_SIZE, img.size[0], y*GRID_SIZE), fill=(0, 0, 0))

    for x in range(17):
        draw.line((x*GRID_SIZE, 0, x*GRID_SIZE, img.size[1]), fill=(0, 0, 0))

def handleLargeRange(out_pixels, ip_set, non_sig_bits, b1, b2):
    if non_sig_bits <= 24:
        for i in range(b2, b2 + 2**(non_sig_bits-16)):
            addIPs(out_pixels, b1, i, MAX_COUNT)
        return

    for i in range(b1, b1 + 2**(non_sig_bits-24)):
        for j in range(256):
            addIPs(out_pixels, i, j, MAX_COUNT)
    

def addIPs(out_pixels, b1, b2, count):
    out_pixels[b1][b2] += count

def addStringIPs(out_pixels, ip_set):
    match = re.search(IP_regex, ip_set).groups()
    b1 = int(match[0])
    b2 = int(match[1])

    count = 1

    if match[5] is not None:
        non_sig_bits = 32 - int(match[5])
        count = 2**non_sig_bits
        if count > MAX_COUNT:
            handleLargeRange(out_pixels, ip_set, non_sig_bits, b1, b2)
            return
            
    addIPs(out_pixels, b1, b2, count)

def addFile(out_pixels, file_path):
    # print(file_path + ":")
    with open(file_path, "r") as file:
        for line in file.readlines():
            if line[0] == "#":
                continue

            addStringIPs(out_pixels, line.strip())

def addFiles(infile):
    out_pixels = [[0 for _ in range(RES)] for _ in range(RES)]

    if os.path.isdir(infile):
        for f in os.listdir(infile):
            addFile(out_pixels, infile + "/" + f)        
    else:
        addFile(out_pixels, infile)

    return out_pixels

if(len(sys.argv) != 3):
    print(f"Usage: {sys.argv[0]} <input file/folder> <outfile>")
    sys.exit(1)

img = newImage()
pixels = addFiles(sys.argv[1])
totalIPCount = drawPixels(img, pixels)
drawOverlay(img)
img.save(sys.argv[2])

print(f"Total IPs blocked = {totalIPCount}")