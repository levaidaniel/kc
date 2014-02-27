#!/bin/sh -e

set -e


echo "test => $0"

case "$(uname -s)" in
	Linux|CYGWIN*)
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

SHA1_INITIAL=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)

printf "new ABCDEF\nGHIJKL\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

printf "swap 0 3\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '1e43bcd8d5b67176d0582a1cbbb2fe11c02e3672' ];then
	echo "$0 test ok (swap)!"
else
	echo "$0 test failed (swap)!"
	exit 1
fi

printf "swap 0 3\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '374b7bbc7e4f120740863e6e52b31a50c28948ab' ];then
	echo "$0 test ok (swap back)!"
else
	echo "$0 test failed (swap back)!"
	exit 1
fi

printf "del 3\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = "$SHA1_INITIAL"  ];then
	echo "$0 test ok (swap initial state)!"
else
	echo "$0 test failed (swap initial state)!"
	exit 1
fi

exit 0
