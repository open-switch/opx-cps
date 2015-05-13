#!/usr/bin/python

import cps
import os
import cps_utils

if __name__ == '__main__':

    cps.config('10','2.3','pyobj','',True,"node")
    cps.config('11','2.3.1','pyobj/node','',False,"node")
    cps.config('12','2.3.2','pyobj/time','',False,"node")

    l = []
    if cps.get([{'key':'1.2.3.4.5','data':{}}],l) == True:
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

