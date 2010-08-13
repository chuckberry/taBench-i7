#!/bin/bash

function print_usage {
cat - <<EOF

$1 --> Makefile label
$2 --> binaries destination path
$3 --> [optional] monitor attributes, that are function traced nr of sample

EOF
}

LABEL_IN="$1"
PATH_BIN="$2"
MONITOR_ATTRIB="$3"

if [[ x$LABEL_IN == "x" || x$PATH_BIN == "x" ]]; then
	print_usage
	exit 1;
fi

LABEL=`cat Makefile | grep "$LABEL_IN:"`
if [ x$LABEL == "x" ]; then
	echo "Makefile label not exists"
	exit 1;
fi

BINARIES=`ls -l | grep *KB`
if [ x$BINARIES != "x" ]; then
	make clean
fi

make $1
cp nwBench-* "$2/bmarks"
cp monitor-* "$2/bmarks"
make clean

cd $2
echo "******** compilation of `date` ***************" >> compile_log.txt
echo "label used: $1" >> compile_log.txt
echo "monitor attrib: $3" >> compile_log.txt
echo "******** end report of `date`  ***************" >> compile_log.txt
