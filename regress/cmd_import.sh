#!/bin/sh -e

set -e


echo "test => $0"

if [ ! -r regress/test_export.kcd ];then
	echo "$0 test failed (unreadable export file)!"
	exit 1
fi


PASSWORD=aabbccdd112233


printf "del 0\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
printf "del 0\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
printf "append -k regress/test_export.kcd -P bcrypt -m cfb\n${PASSWORD}\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '4177f5737e79120a9419772447f201221245e541' ];then
	echo "$0 test ok (append)!"
else
	echo "$0 test failed (append)!"
	exit 1
fi


printf "del 0\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
printf "del 0\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
# Also fake an invalid parameter
printf "import --help\nimport -k regress/test_export.kcd -P bcrypt -m cfb\n${PASSWORD}\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = "$SHA1_COMMON_1" ];then
	echo "$0 test ok (import)!"
else
	echo "$0 test failed (import)!"
	exit 1
fi


printf "del 0\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
printf "del 0\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
printf "appendxml -k regress/test_dump.xml\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '4177f5737e79120a9419772447f201221245e541' ];then
	echo "$0 test ok (appendxml)!"
else
	echo "$0 test failed (appendxml)!"
	exit 1
fi


printf "del 0\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
printf "del 0\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
printf "importxml -k regress/test_dump.xml\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = "$SHA1_COMMON_1" ];then
	echo "$0 test ok (importxml)!"
else
	echo "$0 test failed (importxml)!"
	exit 1
fi

exit 0
