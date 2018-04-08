#!/usr/bin/env python2

import numpy as np
import scipy.misc, sys

def munge(im):
    sz  = min(im.shape[:2])                             # Get shorter extent.
    ofs = [(int(x) - sz) // 2 for x in im.shape[:2]]    # Calculate crop corner pixel.
    im  = im[ofs[0]:ofs[0]+sz, ofs[1]:ofs[1]+sz, :]     # Crop to a square.
    im  = scipy.misc.imresize(im, (224, 224))           # Resize to 224 x 224.
    im  = im.astype(np.float32)                         # Convert to FP32.
    im -= [[[103.939, 116.779, 123.68]]]                # Subtract VGG training set mean from every pixel.
    im  = im.transpose((2,0,1))                         # Transpose to channel-major order.
    return im

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python imgconv.py <input file> <output file>")
        exit(1)

    im = scipy.misc.imread(sys.argv[1], mode='RGB')
    im = munge(im)
    im.tofile(sys.argv[2])
