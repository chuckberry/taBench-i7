#!/bin/bash

function print_usage {
cat - <<EOF

$1 --> Makefile label
$2 --> binaries destination path
$3 --> monitor attributes, that are function traced nr of sample
$4 --> suffix for bmarks folder

EOF
}

LABEL_IN="$1"
PATH_BIN="$2"
MONITOR_ATTRIB="$3"
BIN_FOLDER="bmarks-$4"

if [[ x$LABEL_IN == "x" || x$MONITOR_ATTRIB == "x" || x$PATH_BIN == "x" || x$BIN_FOLDER == "x" ]]; then
	print_usage
	exit 1;
fi

if [[ -d "$PATH_BIN/$BIN_FOLDER" ]]; then
	echo "$BIN_FOLDER exists"
	exit 1;
fi

LABEL=`cat Makefile | grep "$LABEL_IN:"`
if [[ x$LABEL == "x" ]]; then
	echo "Makefile label not exists"
	exit 1;
fi

BINARIES=`ls -l | grep *KB`
if [ x$BINARIES != "x" ]; then
	make clean
fi

mkdir $PATH_BIN/$BIN_FOLDER

make $LABEL_IN
cp nwBench-* "$PATH_BIN/$BIN_FOLDER"
cp monitor-* "$PATH_BIN/$BIN_FOLDER"
make clean

cd $PATH_BIN
echo "******** compilation of `date` ***************" >> compile_log.txt
echo "label used: $LABEL_IN" >> compile_log.txt
echo "monitor attrib: $MONITOR_ATTRIB" >> compile_log.txt
echo "******** end report of `date`  ***************" >> compile_log.txt
