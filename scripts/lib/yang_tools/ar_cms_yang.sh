#!/bin/bash
#* Copyright (c) 2019 Dell Inc.
#*
#* Licensed under the Apache License, Version 2.0 (the "License"); you may
#* not use this file except in compliance with the License. You may obtain
#* a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#*
#* THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
#* CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
#* LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
#* FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#*
#* See the Apache Version 2.0 License for specific language governing
#* permissions and limitations under the License.


if [ "$TOOL_ROOT"b = b ] ; then
    TOOL_ROOT=$(dirname $0)/py
fi

#output dir for headers
od=$2

#output dir for sources
od_src=$3

#the yang file to parse
yf=$1

if [ ! -f $TOOL_ROOT/yin_parser.py ] ; then
    echo "Please set TOOL_ROOT to be the directory containing the yin parser"
    exit 1
fi

if [ "$od"b = b ] ; then
    echo "Please pass in a valid out directory"
    echo "$0 [yang file] [output directory]"
    echo "eg. $0 dell-acl.yang /tmp"
    exit 1
fi

if [ -z $YANG_PATH ] ; then
    echo "Please set YANG_PATH to a : separated list of directories containing the Yang files"
    exit 1
fi

if [ ! -d history ] ; then
    mkdir -p history
fi

export YANG_MODPATH=$YANG_PATH
python $TOOL_ROOT/yin_parser.py file=$yf cmsheader=$od cmssrc=$od_src output=cms history=history
if [ ! $? = 0 ] ; then
    rm $of
    exit 1
fi
exit 0
