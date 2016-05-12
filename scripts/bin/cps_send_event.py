#!/usr/bin/python
#
# Copyright (c) 2015 Dell Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN #AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
#  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#

import sys
import cps
import cps_object
import cps_utils

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print "Missing args...."
        print "Usage: cps_send_event.py [operation] [qualifier] [object_id] attr1=value attr2=value"
        print "operation=set/create/delete, qualifier=target/observed/realtime/proposed"
        print "Example: cps_send_event set observed base-pas/media slot=1 port=1 type=61"
        exit(1)

    handle = cps.event_connect()
    print " sending event for...."
    _obj = cps_object.CPSObject(qual=sys.argv[2], module=sys.argv[3])

    for e in sys.argv[4:]:
        attr = e.split('=',1)
        _obj.add_attr(attr[0],attr[1])

    cur_obj = _obj.get()
    ev = {'operation':sys.argv[1], 'key':cur_obj['key'], 'data':cur_obj['data']}
    cps.event_send(handle, ev)
    if 'operation' in ev:
        print "Operation : ", ev['operation']
    cps_utils.print_obj(cur_obj)
