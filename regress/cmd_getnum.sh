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

printf "0\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "0\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'e8e99293c6d1adf5980bbad56b4967def2aa2797' ];then
	echo "$0 test ok (existing)!"
else
	echo "$0 test failed (existing)!"
	exit 1
fi

printf "2\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "2\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '812e96292afbdf1b0cebb40a7db6a7ffa2e52dfe' ];then
	echo "$0 test ok (nonexisting)!"
else
	echo "$0 test failed (nonexisting)!"
	exit 1
fi

echo "9999999999999999999999999999999999999999999999999999999999999999" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(echo "9999999999999999999999999999999999999999999999999999999999999999" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '348cf4ac62a76688ad79803ed74b3faeeea84532' ];then
	echo "$0 test ok (too big)!"
else
	echo "$0 test failed (too big)!"
	exit 1
fi

echo "-99999999999999999999999999999999999999999999999999999999999999" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(echo "-9999999999999999999999999999999999999999999999999999999999999999" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '5b466eca55e2e107259f4cc9b23c16bc06cfb753' ];then
	echo "$0 test ok (too small #1)!"
else
	echo "$0 test failed (too small #1)!"
	exit 1
fi

echo "-1" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(echo "-1" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'ceec076e4de962780a23b5160d7038fb41b914ec' ];then
	echo "$0 test ok (too small #2)!"
else
	echo "$0 test failed (too small #2)!"
	exit 1
fi

exit 0
