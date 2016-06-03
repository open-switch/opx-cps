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
import sys
import cps

def yangwalk(yangpath):
    """
    YANG data model walk
    @params[in] yangpath - Input yang path to fetch its first level contents
    @returns None
    """
        
    if yangpath == '':
        # Find the modules
        i = cps.info("")['names'].keys()
        for mod in i:
            print mod
        return


    if cps.type(yangpath)['attribute_type'] == "leaf":
        #Print YANG leaf attributes
        print "Name = ", cps.type(yangpath)['name'], "\nAttribute Type = ", cps.type(yangpath)['attribute_type'], "\nData Type = ", cps.type(yangpath)['data_type'], "\nDescription = ", cps.type(yangpath)['description']
        return


    slashes = cps.type(yangpath)['name'].count("/")
    complist = cps.info(yangpath, False)['names'].keys()

    #Print first level contents of the specified YANG path
    for elem in complist:
        if cps.type(elem)['name'].count("/") == slashes+1:
            print "\n", cps.type(elem)['name']
            print "\tAttribute Type = ", cps.type(elem)['attribute_type']
            if cps.type(elem)['attribute_type'] == "leaf":
                print "\tData Type = ", cps.type(elem)['data_type']
            print "\tDescription = ", cps.type(elem)['description']


if __name__ == "__main__":

    # Usage: python cps_model_info.py [yangpath]
    if len(sys.argv) >= 2:
        yangpath = sys.argv[1]
    else:
        yangpath = ''

    yangwalk(yangpath)

    sys.exit(0)


