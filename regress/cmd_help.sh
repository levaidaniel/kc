#!/bin/sh


echo "test => $0"

if echo "help" |./kc -b -k regress/test -p regress/testpass >/dev/null;then
	echo $0 test ok!
	exit 0
else
	echo $0 test failed!
	exit 1
fi
