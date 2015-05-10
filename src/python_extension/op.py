#!/usr/bin/python

import cps
import os

def search_for_cps_libs():
    libs=[]
    path=os.getenv("LD_LIBRARY_PATH")
    if path==None:
        path='/opt/ngos/lib'
    for i in path.split(':'):
        print "Searching "+i
        files = os.listdir(i)
        for f in files:
            if f.find('cpsclass')==-1:continue
            libs.append(i)
            break;
    for i in libs:
        print "Loading from "+i
        res = cps.init(i,'cpsclass')
        print res

if __name__ == '__main__':
    search_for_cps_libs()
    cps.config('2.3.1','pyobj/node','',False,"node")
    l = []
    if cps.get(['1.2'],l) == True:
        for i in l:
            print i

    single_update = {}
    obj = {'key':'','data':{}}
    single_update['change'] = obj
    single_update['operation'] = "set"
    obj['key'] = '1.2.3.4'
    obj['data'] = { 'pyobj/node':'Cliff' }
    l= [single_update]

    another_update = { 'change': {'key':'1.2.3.5', 'data': {'pyobj/node':'Joe'}},'operation':'create' }
    l.append(another_update)
    cps.transaction(l)
    print l

