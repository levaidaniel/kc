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

rm -f regress/test_export

printf "xport regress/test_export\n" |./kc -b -k regress/test -p regress/testpass

if [ ! -r "regress/test_export" ];then
	echo "$0 test failed (unreadable export file)!"
	exit 1
fi

SHA1=$(cat regress/test_export |sed -e 's/created="[0-9]\+"//' -e 's/modified="[0-9]\+"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '23fa5cccf87201c76a9fd7b29be13cfda3b2295b' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
