import json
import numpy as np
import cv2 as cv


with open("/home/ahi/.local/share/askier/ascii_lut_v0.0.3_Monospace_12.json", "r") as fp:
    font_file = json.load(fp)

print(font_file.keys())
pixmap = np.array(font_file.get('pixmap'))
height = font_file.get('pixmap_heights')[0]
width = font_file.get('pixmap_widths')[0]
pmap_area = height * width
# grid_height = int((len(pixmap) // pmap_area) ** 0.5)
# grid_width = math.ceil((len(pixmap) // pmap_area) / grid_height)
# grid = np.zeros_like((grid_height * height, grid_width * width)) + 127
pmaps = []
pmap_idx = ord('g') - 32
pmap_start = pmap_idx * pmap_area
pmap = pixmap[pmap_start:pmap_start + pmap_area]
pmap = np.array(pmap).reshape((height, width))
print(pmap.min(), pmap.max())
pmap = pmap.astype('uint8')

cv.imshow('pixel map', pmap)
cv.waitKey(0)
# print(len(pmaps))