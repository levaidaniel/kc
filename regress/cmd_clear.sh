#!/bin/sh -e

set -e


echo "test => $0"

if [ $(echo "clear" |./kc -b -k regress/test -p regress/testpass |grep -E -c -e '^$') -eq 50 ];then
	echo $0 test ok!
	exit 0
else
	echo $0 test failed!
	exit 1
fi
