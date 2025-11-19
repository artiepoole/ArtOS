import numpy as np
from PIL import Image

threshold = 50

img_from_file = Image.open("splash_logo.jpg")  # load file

img = np.array(img_from_file).astype(np.uint32)  ## convert type to handle > 255

summed = np.sum(img, 2)  # create a total brightness map using sum
img[summed < threshold] = 0  # set all dark pixels to 0

# convert to correctly ordered u32
im_flat = 2**16 * img[:, :, 0] + 2**8 * img[:, :, 1] + img[:, :, 2]
# save to raw file
im_flat.tofile("splash_logo_thresh")
