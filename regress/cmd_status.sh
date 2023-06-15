#!/bin/sh -e

set -e


echo "test => $0"


database_name=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |grep -E -e '^Database file:' |sed -e 's/ (.*)$//')
xml_size=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |grep -E -e '^XML structure size: ')
ssh_agent=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |grep -E -e '^SSH agent:')
yubikey=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |grep -E -e '^Security key\(s\):')
encryption=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |grep -E -e '^Encryption:')
kdf=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |grep -E -e '^Password function:')
key_length=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |grep -E -e '^Key length:')
read_only=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |grep -E -e '^Read-only: ')
modified=$(echo "status" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |grep -E -e '^Modified: ')

if [ "$database_name" = "Database file: ${KC_DB}" ];then
	echo "$0 test ok (database name)!"
else
	echo "$0 test failed (database name)!"
	exit 1
fi

if [ "$xml_size" = 'XML structure size: 825 bytes' ];then
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

if [ "$ssh_agent" = 'SSH agent: no' ];then
	echo "$0 test ok (ssh agent)!"
else
	echo "$0 test failed (ssh agent)!"
	exit 1
fi

if [ "$yubikey" = 'Security key(s): no' ];then
	echo "$0 test ok (yubikey)!"
else
	echo "$0 test failed (yubikey)!"
	exit 1
fi

if [ "$kdf" = 'Password function: sha512 (100000 iterations)' ];then
	echo "$0 test ok (kdf)!"
else
	echo "$0 test failed (kdf)!"
	exit 1
fi

if [ "$key_length" = 'Key length: 32 bytes / 256 bits' ];then
	echo "$0 test ok (key length)!"
else
	echo "$0 test failed (key length)!"
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
