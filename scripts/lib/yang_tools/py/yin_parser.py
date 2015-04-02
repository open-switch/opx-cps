import yin_model
import os
import sys

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print "Missing arguements need yin formatted file"
    yf = yin_model.CPSYangModel(sys.argv[1]);
    yf.show()
    yf.close()

    sys.exit(0)
