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

printf "list\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "list\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '646c97c66d3c5f0810adfabe230ba164a182825e' ];then
	echo "$0 test ok (list)!"
else
	echo "$0 test failed (list)!"
	exit 1
fi

printf "list 2\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "list nonexistent\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '646c97c66d3c5f0810adfabe230ba164a182825e' ];then
	echo "$0 test ok (list with pager parameter)!"
else
	echo "$0 test failed (list with pager parameter)!"
	exit 1
fi

exit 0
