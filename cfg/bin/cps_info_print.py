#!/usr/bin/python

import cps
import sys

if __name__ == '__main__':
    cps.init('/opt/ngos/lib', 'cpsclass')
    f = cps.info('')
    r = {}
    for i in f.keys():
        r[f[i]] = i
    del sys.argv[0]
    for i in sys.argv:
        if i in f:
            print f[i]
        if i in r:
            print r[i]
