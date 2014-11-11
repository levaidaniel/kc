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

printf "insert 0 3\ninsert 0 3\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '77b86226dc8d39da992241567e0986d647073159' ];then
	echo "$0 test ok (insert)!"
else
	echo "$0 test failed (insert)!"
	exit 1
fi

printf "insert 0 3\ninsert 0 3\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '649e3af4746b4284a4372791a0426d3af4135834' ];then
	echo "$0 test ok (insert back)!"
else
	echo "$0 test failed (insert back)!"
	exit 1
fi

printf "del 3\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = "$SHA1_INITIAL"  ];then
	echo "$0 test ok (insert initial state)!"
else
	echo "$0 test failed (insert initial state)!"
	exit 1
fi

exit 0
