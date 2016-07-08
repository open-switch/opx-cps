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
        print "\nMissing args...."
        print "Usage: cps_send_event.py [operation] [qualifier] [object_id] attr1=value attr2=value"
        print "operation=set/create/delete, qualifier=target/observed/realtime/proposed"
        print "Example1: cps_send_event.py set observed base-pas/media slot=1 port=1 type=61"
        print "Example2: cps_send_event.py create observed  dell-base-if-cmn/if/interfaces-state/interface" \
              + " if/interfaces-state/interface/name=e101-007-0 if/interfaces-state/interface/oper-status=2\n"
        exit(1)

    _in_op = sys.argv[1].lower();
    _in_qual = sys.argv[2].lower();

    _operation = ['set', 'create', 'delete']
    if _in_op not in _operation:
       print "\nCheck operation, supported operations (set/create/delete)\n"
       exit(1)

    _qual = ['target', 'observed', 'proposed', 'realtime']
    if _in_qual not in _qual:
       print "\nCheck qualifier, supported qualifiers (target/observed/proposed/realtime)\n"
       exit(1)

    _key = cps.key_from_name(_in_qual, sys.argv[3])

    if ((_key == "") or (_key  == None)):
        print "\nCheck the object name, not a valid object\n"
        exit(1)

    handle = cps.event_connect()
    print " sending event for...."
    _obj = cps_object.CPSObject(qual=_in_qual, module=sys.argv[3])

    for e in sys.argv[4:]:
        attr = e.split('=',1)
        _obj.add_attr(attr[0],attr[1])

    cur_obj = _obj.get()
    ev = {'operation':_in_op, 'key':cur_obj['key'], 'data':cur_obj['data']}
    cps.event_send(handle, ev)
    if 'operation' in ev:
        print "Operation : ", ev['operation']
    cps_utils.print_obj(cur_obj)
