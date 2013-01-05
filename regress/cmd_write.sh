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

printf "write\n" |./kc -b -k regress/test -p regress/testpass

SHA1=$($SHA1_BIN regress/test |cut -d' ' -f1)
if [ "$SHA1" = '782e45822b5b37c44187ef06e262fe18f3545313' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
