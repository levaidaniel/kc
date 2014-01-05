#!/bin/sh -e

set -e


echo "test => $0"


database_name=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |grep -E -e '^Database file:' |sed -e 's/ (.*)$//')
xml_size=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |grep -E -e '^XML structure size: ')
encryption=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |grep -E -e '^Encryption:')
kdf=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |grep -E -e '^Password handling:')
read_only=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |grep -E -e '^Read-only: ')
modified=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |grep -E -e '^Modified: ')

if [ "$database_name" = "Database file: ${KC_DB}" ];then
	echo "$0 test ok (database name)!"
else
	echo "$0 test failed (database name)!"
	exit 1
fi

if [ "$xml_size" = 'XML structure size: 502 bytes' ];then
	echo "$0 test ok (xml size)!"
else
	echo "$0 test failed (xml size)!"
	exit 1
fi

if [ "$encryption" = 'Encryption: aes256, cbc' ];then
	echo "$0 test ok (cipher and mode)!"
else
	echo "$0 test failed (cipher and mode)!"
	exit 1
fi

if [ "$kdf" = 'Password handling: sha512' ];then
	echo "$0 test ok (kdf)!"
else
	echo "$0 test failed (kdf)!"
	exit 1
fi

if [ "$read_only" = 'Read-only: no' ];then
	echo "$0 test ok (readonly)!"
else
	echo "$0 test failed (readonly)!"
	exit 1
fi

if [ "$modified" = 'Modified: no' ];then
	echo "$0 test ok (modified)!"
else
	echo "$0 test failed (modified)!"
	exit 1
fi

exit 0
