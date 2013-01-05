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

SHA1=$($SHA1_BIN regress/test |cut -d' ' -f1)
if [ "$SHA1" = '479b140016d3bbddb230caa7baa3795d0f89b096' ];then
	echo "$0 test ok (copy)!"
else
	echo "$0 test failed (copy)!"
	exit 1
fi

printf "c emptychain\nmove 0 default\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA1=$($SHA1_BIN regress/test |cut -d' ' -f1)
if [ "$SHA1" = 'c98c896389f83362ac9d5004ec1a018ea5f7cf39' ];then
	echo "$0 test ok (move)!"
else
	echo "$0 test failed (move)!"
	exit 1
fi

exit 0
