#!/usr/bin/python

__copyright__ = ''' Copyright (c) 2015 Dell Inc. '''
__license__ = '''
*
* Licensed under the Apache License, Version 2.0 (the "License"); you may
* not use this file except in compliance with the License. You may obtain
* a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
*
* THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
* CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
* LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
* FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
*
* See the Apache Version 2.0 License for specific language governing
* permissions and limitations under the License.
'''

import cps
import sys

if __name__ == '__main__':
    cps.init('/opt/ngos/lib', 'cpsclass')
    f = cps.info('')
    r = {}
    for i in f.keys():
        r[f[i]] = i
    del sys.argv[0]
    for i in sys.argv:
        if i in f:
            print f[i]
        if i in r:
            print r[i]
