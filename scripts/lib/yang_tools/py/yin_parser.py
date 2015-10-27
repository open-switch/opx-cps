import yin_model
import os
import sys

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print "Missing arguements need yin formatted file"

    d = {}

    header = None
    src = None

    for i in sys.argv:
        if i.find('=') != -1:
            key, value = i.split('=')
            d[key] = value
    yf = yin_model.CPSYangModel(d)
    yf.close()

    sys.exit(0)
