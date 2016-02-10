#!/usr/bin/python
#
# Copyright (c) 2016 Dell Inc.
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

import cps
import os
import cps_utils

if __name__ == '__main__':

    cps.config('10', '2.3', 'pyobj', '', True, "node")
    cps.config('11', '2.3.1', 'pyobj/node', '', False, "node")
    cps.config('12', '2.3.2', 'pyobj/time', '', False, "node")

    l = []
    if cps.get([{'key': '1.2.3.4.5', 'data': {}}], l):
        for i in l:
            print i

    single_update = {}
    obj = {'key': '', 'data': {}}
    single_update['change'] = obj
    single_update['operation'] = "set"
    obj['key'] = '1.2.3.4'
    obj['data'] = {'pyobj/node': 'Cliff'}
    l = [single_update]

    another_update = {
        'change': {
            'key': '1.2.3.5',
            'data': {
                'pyobj/node': 'Joe'}},
        'operation': 'create'}
    l.append(another_update)
    cps.transaction(l)
    print l
