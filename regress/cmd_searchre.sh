#!/bin/sh -e

set -e


echo "test => $0"

printf "/ ^d.*.[abck]ey[0-9]$\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "/ ^d.*.[abck]ey[0-9]+$\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '040b16edbb11c9e9e3da9c09389000a34d473a6a' ];then
	echo "$0 test ok (current chain)!"
else
	echo "$0 test failed (current chain)!"
	exit 1
fi

printf "/i ^d.*.[ABCK]EY[0-9]$\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "/i ^d.*.[ABCK]EY[0-9]+$\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '040b16edbb11c9e9e3da9c09389000a34d473a6a' ];then
	echo "$0 test ok (current chain, ignore case)!"
else
	echo "$0 test failed (current chain, ignore case)!"
	exit 1
fi

printf "/! ^d.*.[abck]ey[1-9]$\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "/! ^d.*.[abck]ey[1-9]+$\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '2f76d24ccb6ddabe20226c426370f8f2027e38b0' ];then
	echo "$0 test ok (current chain, inverse)!"
else
	echo "$0 test failed (current chain, inverse)!"
	exit 1
fi

printf "/* ^(default)*[abck]ey(test)*[0-9]$\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "/* ^(default)*[abck]ey(test)*[0-9]$\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'bed82c25199d5e6a22f843fa7859cadabb147cbc' ];then
	echo "$0 test ok (all chains)!"
else
	echo "$0 test failed (all chains)!"
	exit 1
fi

printf "/!* ^(default)*[abck]ey(test)*[1-9]$\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "/!* ^(default)*[abck]ey(test)*[1-9]$\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '0a0c9d55e2809052c23c607ed581eba6dc3295a5' ];then
	echo "$0 test ok (all chains, inverse)!"
else
	echo "$0 test failed (all chains, inverse)!"
	exit 1
fi

printf "/!*i ^(default)*[ABCK]EY(tESt)*[1-9]$\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "/!*i ^(default)*[ABCK]EY(tESt)*[1-9]$\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '0a0c9d55e2809052c23c607ed581eba6dc3295a5' ];then
	echo "$0 test ok (all chains, inverse, ignore case)!"
else
	echo "$0 test failed (all chains, inverse, ignore case)!"
	exit 1
fi

printf "/ nonexistent\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "/ nonexistent\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '0fe706a810deffbafd203d78867e620c6bc2677f' ];then
	echo "$0 test ok (nonexistent)!"
else
	echo "$0 test failed (nonexistent)!"
	exit 1
fi

printf "c/ chain\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "c/ chain\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'd04c0b373b429a0ef466e25be38946e5dde1e915' ];then
	echo "$0 test ok (c/)!"
else
	echo "$0 test failed (c/)!"
	exit 1
fi

printf "c/i chain\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "c/i chain\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'd04c0b373b429a0ef466e25be38946e5dde1e915' ];then
	echo "$0 test ok (c/i, ignore case)!"
else
	echo "$0 test failed (c/i, ignore case)!"
	exit 1
fi

printf "c/! chain\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "c/! chain\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '205d065455c5977fea18fdb8521b87151503cec0' ];then
	echo "$0 test ok (c/, inverse)!"
else
	echo "$0 test failed (c/, inverse)!"
	exit 1
fi

exit 0
