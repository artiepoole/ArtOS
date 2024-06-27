import numpy as np
from PIL import Image

img_from_file = Image.open('splash.png')
img = np.array(img_from_file).astype(np.uint32)
im_flat = 2 ** 16 * img[:, :, 0] + 2 ** 8 * img[:, :, 1] + img[:, :, 2]
np.savetxt('splash_data.txt', im_flat, fmt='0x%0.6X', newline=' \n', delimiter=' ')
