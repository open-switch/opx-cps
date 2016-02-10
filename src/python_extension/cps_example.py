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
import sys
import os

if __name__ == '__main__':
    # load the mapping file
    cps.init(
        '/localdisk/cwichmann/cps-api/workspace/debian/jessie/x86_64/sysroot/opt/ngos/lib/',
        'libcpsclass-')

    data = cps.info('19')
    print data
    data = cps.info('20')
    print data
    print cps.convdict({'1': 'dasra'})
    # Get the cps objects
    result = cps.get(['1.2.3.4'])

    # print discovered map
    print result
    # walk through the map and print the contents
    for d in result.keys():
        print d
        # convert to displayable info
        di = cps.arrayconv(d, result[d])
        cps.dictconv(d)
