#!/bin/bash

if [ "$TOOL_ROOT"b = b ] ; then
    TOOL_ROOT=$PWD/py
fi

od=$2

yf=$1
tf=$od/$(basename $1 .yang).yin
of=$od/$(basename $1 .yang).h

if [ ! -f $TOOL_ROOT/ar_yin_reader.py ] ; then
    echo "Please set TOOL_ROOT to be the directory containing the yin parser"
    exit 1
fi

if [ "$od"b = b ] ; then
    echo "Please pass in a valid out directory"
    echo "$0 [yang file] [output directory]"
    echo "eg. $0 dell-acl.yang /tmp"
    exit 1
fi

pyang $yf -f yin > $tf

python $TOOL_ROOT/ar_yin_reader.py $tf > $of
