#!/bin/sh -e

set -e


echo "test => $0"

printf "cnew newchain\ndescription\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '639954900f4621e8d7743cba57b32cddd2327bd2' ];then
	echo "$0 test ok! (got name)"
else
	echo "$0 test failed! (got name)"
	exit 1
fi

printf "cnew\nnewchain2\ndescription2\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '1b44fc68a532ad4226ba930ce0257b78e19f4917' ];then
	echo "$0 test ok! (prompt for name)"
else
	echo "$0 test failed! (prompt for name)"
	exit 1
fi


exit 0
