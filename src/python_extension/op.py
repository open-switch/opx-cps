#!/usr/bin/python

import cps

if __name__ == '__main__':
    cps.config('2.3.1','pyobj/node','',False,"node")
    results = {}
    if cps.get(['1.2.3.4'],results) == True:
        print results

    single_update = {}
    obj = {}
    single_update['change'] = obj
    single_update['operation'] = "set"
    obj['key'] = '1.2.3.4'
    obj_data={}
    obj['data'] = obj_data
    obj_data['node'] = "Cliff"
    l= [single_update]

    another_update={ 'change': {'key':'1.2.3.4', 'data': {'node':'Joe'}},'operation':'create' }
    l.append(another_update)
    cps.transaction(l)
    print l

