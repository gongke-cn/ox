#!/bin/bash
if [ "x$VALGRIND" != "x" ]; then VALGRIND="valgrind -s --leak-check=full"; fi
EXE=`echo $0|sed "s+\.sh$++"`
DIR=`dirname $EXE`
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DIR/../%LIB_ARCH% $VALGRIND $EXE "$@"
