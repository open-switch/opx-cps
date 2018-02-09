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
import cps_utils
import cps_object

if __name__ == '__main__':
    _obj = cps_object.object_from_parameters(prog='cps_get_oid', description='This process \
                    will perform a CPS get and return the results.  \
        The command line supports getting a single object with many attributes.  \
        An example is: %(prog)s cps/node-group -attr name=\'localhost\'')

    k=[_obj.get()]
    l=[]

    _cur_key = ''
    cps.get(k, l)
    for entry in l:
        o = cps_object.CPSObject(obj=entry)
        if (_cur_key!=o.get_key()):
            _cur_key = o.get_key()
            print('\n============%s==========\n' % cps.name_from_key(_cur_key,1))

        cps_utils.print_obj(o.get(),show_key=False)
        print "------------------------------------------------"
