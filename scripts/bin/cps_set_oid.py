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
import time

import argparse

if __name__ == '__main__':

    _obj = cps_object.object_from_parameters(prog='cps_set_oid', description="""Perform a CPS \
    commit operation taking the object specified on the command line.
    An example would be:
    %(prog)s target delete if/interfaces/interface name=e001""",optional_fields=['oper','commit-event'])

    ch = {'operation': _obj.get_property('oper'), 'change': _obj.get()}

    if cps.transaction([ch]):
        print "Success"
        cps_utils.print_obj(ch['change'])
