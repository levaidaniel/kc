#!/bin/sh -e

set -e


echo "test => $0"

printf "list\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "list\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '040b16edbb11c9e9e3da9c09389000a34d473a6a' ];then
	echo "$0 test ok (list)!"
else
	echo "$0 test failed (list)!"
	exit 1
fi

exit 0
