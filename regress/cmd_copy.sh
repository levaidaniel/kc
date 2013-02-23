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

printf "copy 0 emptychain\nwrite\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '85c11fd9d14afc76c0bb5045cb0ef1566b284e07' ];then
	echo "$0 test ok (copy)!"
else
	echo "$0 test failed (copy)!"
	exit 1
fi

printf "c emptychain\nmove 0 default\nwrite\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '372f679706efd7977fce1a2abb05303f694e59da' ];then
	echo "$0 test ok (move)!"
else
	echo "$0 test failed (move)!"
	exit 1
fi

exit 0
