#!/usr/bin/python

import cps

if __name__ == '__main__':
    l=[]
    another_update={ 'change': {'key':'1.2.3.4', 'data': {'node':'Joe'}},'operation':'create' }
    l.append(another_update)
    cps.transaction(l)
    print l

