#!/usr/bin/python

import sys
import cps
import cps_utils
import cps_object

if __name__ == '__main__':
    if len(sys.argv)==1:
        print "Missing args.  Please ensure you enter a request in the following format:"
        print " %s operation class-name param=value where... " %sys.argv[0]
        print "operation = set,delete,create,rpc"
        print "class-name = a cps class name use cps.info('',True) to get a full system list of classes"
        print "param=value a parameter to set/change/filter on"
        print "%s operation base-port/physical hardware-port-id=26 admin-state=2" %sys.argv[0]
        exit(1)

    ch = { 'operation' : sys.argv[1], 'change' : {}}

    cur_obj = cps_object.CPSObject(sys.argv[2])

    for e in sys.argv[3:]:
        res = e.split('=',1)
        cur_obj.add_attr(res[0],res[1])

    ch['change'] = cur_obj.get()

    if cps.transaction([ch]):
        print "Success"
        cps_utils.print_obj(ch['change'])

