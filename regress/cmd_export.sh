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

rm -f regress/test_export.kcd

PASSWORD=aabbccdd112233

printf "export -k regress/test_export -P bcrypt -m cfb128\n${PASSWORD}\n${PASSWORD}\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

if [ ! -r "regress/test_export.kcd" ];then
	echo "$0 test failed (unreadable export file)!"
	exit 1
fi

if printf "${PASSWORD}" |${KC_RUN} -b -k regress/test_export.kcd -P bcrypt -m cfb128;then
	echo "$0 test ok (export)!"
else
	echo "$0 test failed (export)!"
	exit 1
fi


rm -f regress/test_dump.xml

printf "dump -k regress/test_dump\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

if [ ! -r "regress/test_dump.xml" ];then
	echo "$0 test failed (unreadable dump file)!"
	exit 1
fi

SHA1=$(cat regress/test_dump.xml |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'f7fb5577878b3d1ceaf8aa14ea9c273a36b03d15' ];then
	echo "$0 test ok (dump)!"
else
	echo "$0 test failed (dump)!"
	exit 1
fi

exit 0
