#!/bin/sh

if [ "x$1" == "x" ] ; then
    exit 1
fi
if [ "x$2" == "x" ] ; then
    exit 1
fi
if [ "x$3" == "x" ] ; then
    exit 1
fi

awk -f $1 $2 > $3
