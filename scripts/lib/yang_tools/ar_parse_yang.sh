#!/bin/bash

if [ "$TOOL_ROOT"b = b ] ; then
    TOOL_ROOT=$(dirname $0)/py
fi

#output dir for headers
od=$2

#output dir for sources
od_src=$3

#the yang file to parse
yf=$1


of=$od/$(basename $1 .yang).h
of_src=$od_src/$(basename $1 .yang).cpp

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
export YANG_MODPATH=$YANG_PATH
python $TOOL_ROOT/yin_parser.py file=$yf cpsheader=$of cpssrc=$of_src output=cps history=history
if [ ! $? = 0 ] ; then
    rm $of
    exit 1
fi
exit 0
