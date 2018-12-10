#!/usr/bin/python
#
# Copyright (c) 2018 Dell Inc.
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
import sys

if __name__ == '__main__':

    del sys.argv[0]  #delete this program's name from the args

    for __arg in sys.argv:
        __obj_def = cps.info(__arg)
        if not 'names' in __obj_def:
            print('Failed to translate %s to a object.' % __arg)
            continue
        __names = __obj_def['names'].keys()

        for __name in __names:
            __elem = cps.type(__name)

            if 'data_type' not in __elem:
                print('Failed to get object details for %s' % __name)
                continue
            print('Name: %s\nData Type: %s\nID: %s\nAtrribute Type: %s\n\n' \
                  % (__elem['name'],__elem['data_type'],__elem['id'],__elem['attribute_type']) )

