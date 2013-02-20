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

printf "0\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "0\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '087c18b2c195235487f9340232b3e1a3d1321fc8' ];then
	echo "$0 test ok (existing)!"
else
	echo "$0 test failed (existing)!"
	exit 1
fi

printf "2\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "2\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '6e9d4f5fb288d38a9d5e849be38100e08389689f' ];then
	echo "$0 test ok (nonexisting)!"
else
	echo "$0 test failed (nonexisting)!"
	exit 1
fi

exit 0
