#!/bin/sh -e

set -e


echo "test => $0"

if echo "help" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} >/dev/null;then
	echo $0 test ok!
	exit 0
else
	echo $0 test failed!
	exit 1
fi
