import yin_model
import os
import sys

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print "Missing arguements need yin formatted file"
    print "YANG_PATH = "+os.environ.get('YANG_PATH',"")

    yf = yin_model.CPSYangModel(sys.argv[1]);
    yf.show()
    yf.close()

    sys.exit(0)
