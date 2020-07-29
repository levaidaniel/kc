#!/bin/sh -e

set -e


echo "test => $0"

printf "c testchain\nlist\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "c testchain\nlist\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<(default|testchain)% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "${SHA1}" = '1886fcb9e486061976c72ab5e76de444bc1133c3' ];then
	echo "$0 test ok (change chain)!"
else
	echo "$0 test failed (change chain)!"
	exit 1
fi

printf "cnew 10\ndescription\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
printf "c 10\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "c 10\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "${SHA1}" = "${SHA1_KEYCHAIN_NOT_FOUND}" ];then
	echo "$0 test ok (change chain #2)!"
else
	echo "$0 test failed (change chain #2)!"
	exit 1
fi

printf "cc 10\nlist\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "cc 10\nlist\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<(default|10)% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "${SHA1}" = '373d31f0b14700a8dc3199404d069b8a2e80a2f6' ];then
	echo "$0 test ok (change chain (cc))!"
else
	echo "$0 test failed (change chain (cc))!"
	exit 1
fi

printf "ccdel 10\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}


printf "c -1\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "c -1\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "${SHA1}" = "${SHA1_KEYCHAIN_NOT_FOUND}" ];then
	echo "$0 test ok (change nonexistent '-1' chain #1)!"
else
	echo "$0 test failed (change nonexistent '-1' chain #1)!"
	exit 1
fi

printf "cnew -1\ndescription\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
printf "c -1\nlist\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "c -1\nlist\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<(default|-1)% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "${SHA1}" = '373d31f0b14700a8dc3199404d069b8a2e80a2f6' ];then
	echo "$0 test ok (change '-1' chain #2)!"
else
	echo "$0 test failed (change '-1' chain #2)!"
	exit 1
fi
printf "ccdel -1\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}


printf "c 999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "c 999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "${SHA1}" = "${SHA1_KEYCHAIN_NOT_FOUND}" ];then
	echo "$0 test ok (nonexistent too big chain)!"
else
	echo "$0 test failed (nonexistent too big chain)!"
	exit 1
fi

printf "c -999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "c -999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "${SHA1}" = "${SHA1_KEYCHAIN_NOT_FOUND}" ];then
	echo "$0 test ok (nonexistent too small chain)!"
else
	echo "$0 test failed (nonexistent too small chain)!"
	exit 1
fi


printf "c nonexistent\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "c nonexistent\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "${SHA1}" = "${SHA1_KEYCHAIN_NOT_FOUND}" ];then
	echo "$0 test ok (nonexistent chain)!"
else
	echo "$0 test failed (nonexistent chain)!"
	exit 1
fi

exit 0
