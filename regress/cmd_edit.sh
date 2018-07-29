#!/bin/sh -e

set -e


echo "test => $0"

if [ ${READLINE} ];then
	cmd="edit 2\nedited_\nedited_\nwrite\n"
else
	cmd="edit 2\nedited_newkey\nedited_newval\nwrite\n"
fi

printf "${cmd}" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = "$SHA1_COMMON_1" ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
