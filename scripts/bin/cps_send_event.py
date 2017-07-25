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

import cps
import cps_object
import cps_utils

if __name__ == '__main__':
    _obj = cps_object.object_from_parameters(prog='cps_send_event.py', description="""Send a CPS event taking the object specified on the command line.
    An example would be: %(prog)s -oper=create observed/dell-base-if-cmn/if/interfaces-state/interface \
        if/interfaces-state/interface/name=e101-007-0 if/interfaces-state/interface/oper-status=2""",optional_fields=['oper'])

    handle = cps.event_connect()
    print " Sending event for...."

    cur_obj = _obj.get()
    ev = {'operation':_obj.get_property('oper'), 'key':cur_obj['key'], 'data':cur_obj['data']}
    cps.event_send(handle, ev)
    if 'operation' in ev:
        print "Operation : ", ev['operation']
    cps_utils.print_obj(cur_obj)
