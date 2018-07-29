#!/bin/sh -e

set -e


echo "test => $0"

printf "cdel newchain2\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
printf "cdel renamed_newchain\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = "$SHA1_COMMON_1" ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
