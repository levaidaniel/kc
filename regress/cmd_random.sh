#!/bin/sh -e

set -e


echo "test => $0"

if printf "random 19\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |tail -n1 |grep -E -e '[[:print:]]{19,19}';then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
