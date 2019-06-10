#
# Copyright (c) 2019 Dell Inc.
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
import time


def get_cb(methods, params):
    print "Get..."

    d = {'key': params['filter']['key'] + "2", 'data': {'pyobj/node': 'Cliff'}}
    params['list'].append(d)
    d = {'key': params['filter']['key'] + "3", 'data': {'pyobj/node': 'Cliff'}}
    params['list'].append(d)

    print params
    return True


def trans_cb(methods, params):
    print "Trans..."
    print params['operation']

    if params['operation'] == 'set':
        params['change']['data']['pyobj/node'] = "Clifford"
        params['change']['data']['pyobj/time'] = time.asctime()
    print params
    return True

if __name__ == '__main__':
    handle = cps.obj_init()
    d = {}
    d['get'] = get_cb
    d['transaction'] = trans_cb

    cps.obj_register(handle, '1.2.3.4', d)
    cps.config('10', '2.3', 'pyobj', '', True, "node")
    cps.config('11', '2.3.1', 'pyobj/node', '', False, "node")
    cps.config('12', '2.3.2', 'pyobj/time', '', False, "node")
    while True:
        time.sleep(1)
