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


PASSWORD=aabbccdd112233

rm -f regress/test_export.kcd
printf "export -k regress/test_export -e blowfish -m ecb\n${PASSWORD}\n${PASSWORD}\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

if [ ! -r "regress/test_export.kcd" ];then
	echo "$0 test failed (unreadable export file, blowfish)!"
	exit 1
fi

if printf "${PASSWORD}" |${KC_RUN} -b -k regress/test_export.kcd -e blowfish -m ecb;then
	echo "$0 test ok (export, blowfish)!"
else
	echo "$0 test failed (export, blowfish)!"
	exit 1
fi


rm -f regress/test_export.kcd
printf "export -k regress/test_export -P bcrypt -e blowfish -m cbc\n${PASSWORD}\n${PASSWORD}\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

if [ ! -r "regress/test_export.kcd" ];then
	echo "$0 test failed (unreadable export file, bcrypt + blowfish)!"
	exit 1
fi

if printf "${PASSWORD}" |${KC_RUN} -b -k regress/test_export.kcd -P bcrypt -e blowfish -m cbc;then
	echo "$0 test ok (export, bcrypt + blowfish)!"
else
	echo "$0 test failed (export, bcrypt + blowfish)!"
	exit 1
fi


if [ ${SCRYPT} ];then
	rm -f regress/test_export.kcd
	printf "export -k regress/test_export -P scrypt\n${PASSWORD}\n${PASSWORD}\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

	if [ ! -r "regress/test_export.kcd" ];then
		echo "$0 test failed (unreadable export file, scrypt)!"
		exit 1
	fi

	if printf "${PASSWORD}" |${KC_RUN} -b -k regress/test_export.kcd -P scrypt;then
		echo "$0 test ok (export, scrypt)!"
	else
		echo "$0 test failed (export, scrypt)!"
		exit 1
	fi
fi


# This test must be the last one that writes test_export.kcd,
# because the cmd_import.sh test will use this as its input.
rm -f regress/test_export.kcd
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
if [ "$SHA1" = "$SHA1_COMMON_1" ];then
	echo "$0 test ok (dump)!"
else
	echo "$0 test failed (dump)!"
	exit 1
fi

exit 0
