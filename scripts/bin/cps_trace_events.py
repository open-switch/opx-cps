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
import datetime
import signal
from threading import Thread


def cps_trace_usage():
    print ""
    print "Usage: cps_trace_events.py <qualifier> [object]"
    print "   qualifier (mandatory): ", cps.QUALIFIERS
    print "   object (optional): if object is not given all the event related to the qualifier will be captured"
    print "Example: cps_trace_events.py observed base-pas/media OR cps_trace_events.py observed"
    print ""
    print "Example for remote trace:"
    print "    cps_trace_events.py observed/fa-vlan/info cps/object-group/group=dnv"
    print ""

ids = cps.info("", False)['ids']

def _get_key(key):
    k = key.split('.')[-2]
    if k in ids:
        return ids[k]
    return key

def _get_obj_from_input():
    if (len(sys.argv) < 2):
         return None

    _qual_list = cps.QUALIFIERS
    _obj = cps_object.CPSObject(module="cps", qual="target")

    _inp = sys.argv[1].lower();
    _lst = _inp.split(cps.PATH_SEP,1);

    #Only 'qualifier' provided as first argument
    if len(_lst) == 1:
       in_qual = _lst[0]
       if in_qual not in cps.QUALIFIERS:
           print "\nCheck the qualifier, not a valid qualifier "
           return None
       #Second argument is the 'module'
       if len(sys.argv) == 3:
          _key = cps.key_from_name(in_qual, sys.argv[2])
       else:
          _key = cps.key_from_qual(in_qual)
       _obj.set_key(_key)
       print " Registering for " + _key
    else:
        try:
            _obj = cps_object.object_from_parameters(prog='cps_trace_events', description="")
        except ValueError:
            return None
        _key = _obj.get_key()
        print " Registering for object ..."
        cps_utils.print_obj(_obj.get())
    print "----------------------------------"

    if ((_key == "") or (_key == None)):
        print "Check the object name, not a valid object\n"
        return None

    return _obj

def _trace_events(_obj):
    handle = cps.event_connect()

    cps.event_register_object(handle, _obj.get())

    while True:
        ev = cps.event_wait(handle)
        print "@",str(datetime.datetime.now()),"- Event for", _get_key(ev['key'])
        if 'operation' in ev:
            print "Operation : ", ev['operation']
        cps_utils.print_obj(ev)
        print "----------------------------------------------"


if __name__ == '__main__':

    _obj = _get_obj_from_input()
    if _obj is None:
        cps_trace_usage()
        sys.exit(1)

    try:
        evt_th = Thread(target=_trace_events, args=(_obj,))
        evt_th.daemon = True
        evt_th.start()
    except:
        print "Failed to start event monitoring thread"
        sys.exit(1)

    #keep the daemon alive
    try:
        signal.pause()
    except (KeyboardInterrupt, SystemExit):
        sys.exit(0)
