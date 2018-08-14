#!/bin/sh -e

set -e


echo "test => $0"

printf "new newkey\nnewval\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'c75a86df63a39dea50049191fc7da1cf9f971b6b' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
