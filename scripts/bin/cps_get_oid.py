#!/usr/bin/python
#
# Copyright (c) 2015 Dell Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
# LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#

import sys
import cps
import cps_utils
import cps_object

if __name__ == '__main__':
    if len(sys.argv) == 1:
        print "Missing args.  Please enter a CPS key path and then optional attributes/values separated by ="
        print "pass 'db' in the argument list to query the CPS DB"
        print "qual = target,observed,running,startup,realtime,.."
        print "qual is an optional argument if not specified, target is used by default"
        print "Examples:"
        print "    ", sys.argv[0], "observed base-port/physical hardware-port-id=26"
        print "  Query DB:"
        print "    ", sys.argv[0], "target db system/system"
        print "    ", sys.argv[0], "running db if/interfaces/interface"
        exit(1)
    l = []
    k = []
    cur_obj = None
    from_db = False
    qual = "target"
    qual_list = ["target","observed","proposed","realtime","running","startup"]
    for e in sys.argv[1:]:
        if e == 'db':
            from_db = True
            continue
        if e in qual_list:
            qual = e
            continue
        if e.find('=') == -1:
            if (cur_obj is None):
                cur_obj = cps_object.CPSObject(qual=qual,module=e)
            else:
                k.append(cur_obj.get())
                cur_obj = cps_object.CPSObject(qual=qual,module=e)
        else:
            res = e.split('=', 1)
            cur_obj.add_attr(res[0], res[1])

    k.append(cur_obj.get())

    if from_db == False:
        cps.get(k, l)
    else:
        for obj in k:
            cps.db_get(obj, l)

    for entry in l:
        print ""
        cps_utils.print_obj(entry)
        print "----------------------------------------------"
