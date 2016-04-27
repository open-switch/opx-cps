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
import cps_object

if __name__ == '__main__':
    if len(sys.argv) == 1:
        print "Missing args.  Please ensure you enter a request in the following format:"
        print " %s qual operation class-name param=value where... " % sys.argv[0]
        print "qual = target,observed,.."
        print "operation = set,delete,create,rpc"
        print "class-name = a cps class name use cps.info('',True) to get a full system list of classes"
        print "param=value a parameter to set/change/filter on"
        print "%s qual operation base-port/physical hardware-port-id=26 admin-state=2" % sys.argv[0]
        print "qual is an optional argument if not specified, target is used by default"
        print "To add embedded attribute specify the all arguments in comma followed by = and its value"
        print "For eg. base-stg/entry/intf,0,ifindex=17 base-stg/entry/intf,0,state=1"
        exit(1)

    qual_list = ["target","observed","proposed","realtime"]
    if sys.argv[1] in qual_list:
        ch = {'operation': sys.argv[2], 'change': {}}
        cur_obj = cps_object.CPSObject(qual=sys.argv[1],module=sys.argv[3])
        arg_list = sys.argv[4:]
    else:
        ch = {'operation': sys.argv[1], 'change': {}}
        cur_obj = cps_object.CPSObject(qual="target",module=sys.argv[2])
        arg_list = sys.argv[3:
                            ]
    for e in arg_list:
        res = e.split('=', 1)
        # For embedded attribute check if comma seperated attribute list is given
        # then add it as embedded
        embed_attrs = res[0].split(',')
        if len(embed_attrs) == 3:
            cur_obj.add_embed_attr(embed_attrs,res[1])
        else:
            cur_obj.add_attr(res[0], res[1])


    ch['change'] = cur_obj.get()

    if cps.transaction([ch]):
        print "Success"
        cps_utils.print_obj(ch['change'])
