#!/bin/sh -e

set -e


echo "test => $0"

printf "copy 0 emptychain\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '2dbe4f401f8552e07dacd1fd3607959855fea80d' ];then
	echo "$0 test ok (copy)!"
else
	echo "$0 test failed (copy)!"
	exit 1
fi

printf "c emptychain\nmove 0 default\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'e4da14b026cbddc698ae1d55c81fa06e08090b03' ];then
	echo "$0 test ok (move)!"
else
	echo "$0 test failed (move)!"
	exit 1
fi

exit 0
