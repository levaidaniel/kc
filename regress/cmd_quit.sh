#!/bin/sh


echo "test => $0"

if echo "quit" |./kc -b -k regress/test -p regress/testpass;then
	echo $0 test ok!
	exit 0
else
	echo $0 test failed!
	exit 1
fi
