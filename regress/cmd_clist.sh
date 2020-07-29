#!/bin/sh -e

set -e


echo "test => $0"

printf "clist\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database."
SHA1=$(printf "clist\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)

if [ "$SHA1" = '8dd5d12b318f3ff1e48f657f2067a1de901638b7' ];then

	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
