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

import yin_model
import os
import sys

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print "Missing arguements need yin formatted file"

    d = {}

    header = None
    src = None

    for i in sys.argv:
        if i.find('=') != -1:
            key, value = i.split('=')
            d[key] = value
    yf = yin_model.CPSYangModel(d)
    yf.close()

    sys.exit(0)
