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

printf "xport regress/test_export\n" |./kc -b -k regress/test -p regress/testpass

if [ ! -r "regress/test_export" ];then
	echo "$0 test failed (unreadable export file)!"
	exit 1
fi

SHA1=$($SHA1_BIN regress/test_export |cut -d' ' -f1)
if [ "$SHA1" = 'fcd724024dbbab3a99afbd103f3ead5e97fe24b4' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
