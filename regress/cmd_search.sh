#!/bin/sh -e

set -e


echo "test => $0"

printf "search key\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "search key\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '040b16edbb11c9e9e3da9c09389000a34d473a6a' ];then
	echo "$0 test ok (current chain)!"
else
	echo "$0 test failed (current chain)!"
	exit 1
fi

printf "searchi KEY\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "searchi KEY\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '040b16edbb11c9e9e3da9c09389000a34d473a6a' ];then
	echo "$0 test ok (current chain, ignore case)!"
else
	echo "$0 test failed (current chain, ignore case)!"
	exit 1
fi

printf "search! y0\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "search! y0\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'aa6776e0232e8a0a458710cb46f62af8cc3a0caa' ];then
	echo "$0 test ok (current chain, inverse)!"
else
	echo "$0 test failed (current chain, inverse)!"
	exit 1
fi

printf "search* key\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "search* key\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'bed82c25199d5e6a22f843fa7859cadabb147cbc' ];then
	echo "$0 test ok (all chains)!"
else
	echo "$0 test failed (all chains)!"
	exit 1
fi

printf "search!* y1\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "search!* y1\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '3d9d16edab7a59308db38c1f24aa51deb21f6b1b' ];then
	echo "$0 test ok (all chains, inverse)!"
else
	echo "$0 test failed (all chains, inverse)!"
	exit 1
fi

printf "search!*i Y1\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "search!*i Y1\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '3d9d16edab7a59308db38c1f24aa51deb21f6b1b' ];then
	echo "$0 test ok (all chains, inverse, ignore case)!"
else
	echo "$0 test failed (all chains, inverse, ignore case)!"
	exit 1
fi

printf "search nonexistent\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "search nonexistent\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '0fe706a810deffbafd203d78867e620c6bc2677f' ];then
	echo "$0 test ok (nonexistent)!"
else
	echo "$0 test failed (nonexistent)!"
	exit 1
fi

printf "csearch chain\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "csearch chain\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'd04c0b373b429a0ef466e25be38946e5dde1e915' ];then
	echo "$0 test ok (csearch)!"
else
	echo "$0 test failed (csearch)!"
	exit 1
fi

printf "csearchi CHAIN\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "csearchi CHAIN\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'd04c0b373b429a0ef466e25be38946e5dde1e915' ];then
	echo "$0 test ok (csearchi, ignore case)!"
else
	echo "$0 test failed (csearchi, ignore case)!"
	exit 1
fi

printf "csearch! chain\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "csearch! chain\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" -e "^Decrypting\.\.\." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '205d065455c5977fea18fdb8521b87151503cec0' ];then
	echo "$0 test ok (csearch, inverse)!"
else
	echo "$0 test failed (csearch, inverse)!"
	exit 1
fi

exit 0
