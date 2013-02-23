#!/bin/sh -e

set -e


echo "test => $0"

case "$(uname -s)" in
	Linux)
		SHA1_BIN=$(which sha1sum)
	;;
	*BSD)
		SHA1_BIN="$(which sha1) -r"
	;;
	*)
		echo "unknown system."
		exit 1
	;;
esac

printf "c testchain\nlist\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "c testchain\nlist\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<(default|testchain)% >' -e "^Opening '${KC_DB}'" |$SHA1_BIN |cut -d' ' -f1)
if [ "${SHA1}" = '1886fcb9e486061976c72ab5e76de444bc1133c3' ];then
	echo "$0 test ok (change chain)!"
else
	echo "$0 test failed (change chain)!"
	exit 1
fi

printf "cnew 10\ndescription\nwrite\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE}
printf "c 10\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "c 10\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" |$SHA1_BIN |cut -d' ' -f1)
if [ "${SHA1}" = 'f00ecea88ac8e16851779e4230ffd0871c453d40' ];then
	echo "$0 test ok (change chain #2)!"
else
	echo "$0 test failed (change chain #2)!"
	exit 1
fi

printf "cc 10\nlist\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "cc 10\nlist\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<(default|10)% >' -e "^Opening '${KC_DB}'" |$SHA1_BIN |cut -d' ' -f1)
if [ "${SHA1}" = '373d31f0b14700a8dc3199404d069b8a2e80a2f6' ];then
	echo "$0 test ok (change chain (cc))!"
else
	echo "$0 test failed (change chain (cc))!"
	exit 1
fi

printf "ccdel 10\nyes\nwrite\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE}


printf "c nonexistent\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "c nonexistent\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'f00ecea88ac8e16851779e4230ffd0871c453d40' ];then
	echo "$0 test ok (nonexistent chain)!"
else
	echo "$0 test failed (nonexistent chain)!"
	exit 1
fi

exit 0
