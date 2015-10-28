#!/usr/bin/python
import sys
import cps
import cps_utils
import cps_object

if __name__ == '__main__':
    if len(sys.argv) == 1:
        print "Missing args.  Please enter a CPS key path and then optional attributes/values separated by ="
        print "%s base-port/physical hardware-port-id=26"
        exit(1)
    l = []
    k = []
    cur_obj = None
    for e in sys.argv[1:]:
        if e.find('=') == -1:
            if (cur_obj is None):
                cur_obj = cps_object.CPSObject(e)
            else:
                k.append(cur_obj.get())
                cur_obj = cps_object.CPSObject(e)
        else:
            res = e.split('=', 1)
            cur_obj.add_attr(res[0], res[1])

    k.append(cur_obj.get())

    cps.get(k, l)
    for entry in l:
        print ""
        cps_utils.print_obj(entry)
