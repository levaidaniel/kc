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

printf "cnew newchain\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA1=$(printf "list\n" |KC_DEBUG=yes ./kc -b -k regress/test -p regress/testpass |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/created="[0-9]\+"//' -e 's/modified="[0-9]\+"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '521beb993f111f50e3d846d6d6df4377bddb1dcb' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
