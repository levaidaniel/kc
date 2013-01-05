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

printf "new newkey\nnewval\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA1=$($SHA1_BIN regress/test |cut -d' ' -f1)
if [ "$SHA1" = 'ac9249e617f17c243a41fba578d7fabd8dac0d2f' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
