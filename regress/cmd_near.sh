#!/bin/sh -e

set -e


echo "test => $0"

printf "new pos3\npos3\nnew pos4\npos4\nnew pos5\npos5\nnew pos6\npos6\nnew pos7\npos7\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

NEAR0_SHA1=$(printf "near 0\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
NEAR1_SHA1=$(printf "near 1\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
NEAR12_SHA1=$(printf "near 1 2\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
NEAR33_SHA1=$(printf "near 3 3\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
NEAR73_SHA1=$(printf "near 7 3\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
NEAR93_SHA1=$(printf "near 9 3\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)

if [ "$NEAR0_SHA1" = 'e50b1ff17587d994b1221339f92c5f0e6eb9b410' ];then
	echo "$0 test ok (near 0)!"
else
	echo "$0 test failed (near 0)!"
	exit 1
fi

if [ "$NEAR1_SHA1" = 'a4da4ae646200bd3363529520caf4b05fe616a9a' ];then
	echo "$0 test ok (near 1)!"
else
	echo "$0 test failed (near 1)!"
	exit 1
fi

if [ "$NEAR12_SHA1" = 'aa0e1ebc0fbcb796648a738c60c18fd3c487267b' ];then
	echo "$0 test ok (near 1 2)!"
else
	echo "$0 test failed (near 1 2)!"
	exit 1
fi

if [ "$NEAR33_SHA1" = 'ca979ba5b064492a4d95bf0372d205296e104e3a' ];then
	echo "$0 test ok (near 3 3)!"
else
	echo "$0 test failed (near 3 3)!"
	exit 1
fi

if [ "$NEAR73_SHA1" = 'd7f861ecdf6d6f158932744b6f184b5f7280729e' ];then
	echo "$0 test ok (near 7 3)!"
else
	echo "$0 test failed (near 7 3)!"
	exit 1
fi

if [ "$NEAR93_SHA1" = "${SHA1_INVALID_INDEX}" ];then
	echo "$0 test ok (near 9 3)!"
else
	echo "$0 test failed (near 9 3)!"
	exit 1
fi


exit 0
