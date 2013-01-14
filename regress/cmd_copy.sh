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

printf "copy 0 emptychain\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA1=$(printf "list\n" |KC_DEBUG=yes ./kc -b -k regress/test -p regress/testpass |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/created="[0-9]\+"//' -e 's/modified="[0-9]\+"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'b7f61da963add6aef344c6d2760e1b763d9f4374' ];then
	echo "$0 test ok (copy)!"
else
	echo "$0 test failed (copy)!"
	exit 1
fi

printf "c emptychain\nmove 0 default\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA1=$(printf "list\n" |KC_DEBUG=yes ./kc -b -k regress/test -p regress/testpass |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/created="[0-9]\+"//' -e 's/modified="[0-9]\+"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '0e52bb15d3c28b1191c10ba02ab39d36f1af41d5' ];then
	echo "$0 test ok (move)!"
else
	echo "$0 test failed (move)!"
	exit 1
fi

exit 0
