#!/bin/sh -e

set -e


echo "test => $0"

if printf "random 19\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" |tail -n1 |grep -E -e '[[:print:]]{19,19}';then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
