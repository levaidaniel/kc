#!/bin/sh -e

set -e


echo "test => $0"

if [ $(echo "clear" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -c -e '^$') -eq 100 ];then
	echo $0 test ok!
	exit 0
else
	echo $0 test failed!
	exit 1
fi
