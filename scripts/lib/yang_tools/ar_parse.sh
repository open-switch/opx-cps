#!/bin/bash

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

