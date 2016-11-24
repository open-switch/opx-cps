#!/bin/bash
#* Copyright (c) 2015 Dell Inc.
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

if [ $# -le 2 ] ; then
  echo "Missing params... $0 [cfg file] [tmp folder] [out folder]"
  exit 1
fi
DIR=$(dirname $0)

CFG=$(readlink -f $1)
TMP=$(readlink -f $2)
OUT=$(readlink -f $3)

cd $DIR
python py/config_reader.py $CFG $TMP $OUT

