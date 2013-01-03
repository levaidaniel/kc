#!/bin/sh -e

set -e


echo "test => $0"

case "$(uname -s)" in
	Linux)
		SHA256_BIN=$(which sha256sum)
	;;
	*BSD)
		SHA256_BIN="$(which cksum) -r -a sha256"
	;;
	*)
		echo "unknown system."
		exit 1
	;;
esac

printf "search estke\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "search estke\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == 'bc5898cae89a9929df2961cd16e77add3fdeeee0d0509adf1476cbe1a9332c13' ];then
	echo "$0 test ok (current chain)!"
else
	echo "$0 test failed (current chain)!"
	exit 1
fi

printf "copy 0 renamed_testchain\nwrite\n" |./kc -b -k regress/test -p regress/testpass

printf "*search estke\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "*search estke\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == 'ece907215eb17ed87daa271fa7981fc2f5fb17ce35d34f192b18e0f5e00971ea' ];then
	echo "$0 test ok (all chains)!"
else
	echo "$0 test failed (all chains)!"
	exit 1
fi

exit 0
