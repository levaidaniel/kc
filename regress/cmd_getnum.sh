#!/bin/sh -e

set -e


echo "test => $0"

case "$(uname -s)" in
	Linux)
		SHA256_BIN=$(which sha256sum)
	;;
	*BSD)
		SHA256_BIN=$(which cksum)
	;;
	*)
		echo "unknown system."
		exit 1
	;;
esac

printf "0\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "0\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == '72f9bb4899c8e8da8d2d5478d65d88343b8fe93b37124706a7a61cd6ae4bb14b' ];then
	echo "$0 test ok (existing)!"
else
	echo "$0 test failed (existing)!"
	exit 1
fi

printf "1\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "1\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == 'ac3f063955e0f25e4999253212e0d88cbbb19c51a458d470d69b1aa70a166245' ];then
	echo "$0 test ok (nonexisting)!"
else
	echo "$0 test failed (nonexisting)!"
	exit 1
fi

exit 0
