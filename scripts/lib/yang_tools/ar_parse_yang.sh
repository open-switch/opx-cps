#!/bin/bash

if [ "$TOOL_ROOT"b = b ] ; then
    TOOL_ROOT=$(dirname $0)/py
fi

od=$2
yf=$1
of=$od/$(basename $1 .yang).h

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
python $TOOL_ROOT/yin_parser.py $yf > $of
