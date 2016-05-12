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
import cps_utils
if __name__ == '__main__':
    if len(sys.argv) < 3:
        print "Missing args.  Please enter a CPS key path in the format of base-if-phy/physical"
        print "Usage: cps_trace_events.py [qualifier] [object]"
        print "Example: cps_trace_events.py observed base-pas/media"
        exit(1)

    handle = cps.event_connect()
    print " Registering for " + sys.argv[1] + " " + sys.argv[2]
    _key = cps.key_from_name(sys.argv[1], sys.argv[2])
    cps.event_register(handle, _key)
    while True:
        ev = cps.event_wait(handle)
        print ev['key']
        if 'operation' in ev:
            print "Operation : ", ev['operation']
        cps_utils.print_obj(ev)
