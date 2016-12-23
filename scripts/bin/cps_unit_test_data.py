#!/usr/bin/python

import sys
import cps
import cps_object
import cps_utils
import random
import math

def random_ifindex(key,val):
    if key.find('ifindex')==-1:
        return val
    return math.trunc((random.random() * 100))

def _gen_cps_obj(qual, typename, attrs, attr_gen ):
    _obj = cps_object.CPSObject(typename,qual,attrs)
    for _attr in attrs.keys():
        if (attr_gen!=None):
            attrs[_attr] = attr_gen(_attr,attrs[_attr])
    
    #cps_utils.print_obj(_obj.get())
    return _obj.get()


def cps_gen_ut_data( amount, persist=False):
    _keys = [ 'base-ip/ipv6','base-ip/ipv6/address',
              'base-ip/ipv4','base-ip/ipv4/address' ]

    _qual = [ 'target', 'observed' ]
    
    _attrs={'base-ip/ipv6/vrf-id': 1, 'base-ip/ipv6/ifindex':0 }    

    l=[]
    for _k in _keys :
        for _q in _qual:
            for i in range(0,amount):
                l.append(_gen_cps_obj(_q, _k,_attrs,random_ifindex))
    
    if persist==True:
        for _o in l:
            cps.db_commit(_o,None,False)
    return l    


def main():
    random.seed()
    print cps_gen_ut_data(100,True)

if __name__ == '__main__':
    main()
