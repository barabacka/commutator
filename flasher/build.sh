#!/bin/bash

cd Dst
rm -r *

cmake ../Src/
retval1=$?
if [ $retval1 -ne 0 ]; then
    exit $retval1
fi
make
retval2=$?
if [ $retval2 -ne 0 ]; then
    exit $retval2
fi

