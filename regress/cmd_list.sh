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

printf "list\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "list\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == '4f6ab60a1abd21a8d0c16a0fef60a94b95b549e49d5cd716e9f3bc5f2d1f9991' ];then
	echo "$0 test ok (list)!"
else
	echo "$0 test failed (list)!"
	exit 1
fi

printf "list testchain\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "list testchain\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == 'e636704e38de5e30b2f78b1e3a78b670a6022c00582494d43ba1840dba22a515' ];then
	echo "$0 test ok (list with parameter)!"
else
	echo "$0 test failed (list with parameter)!"
	exit 1
fi

printf "list nonexistent\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "list nonexistent\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == '3bb0673128fe11dd7add24efab95349b07c45cea2971b8691e85c69427a6d297' ];then
	echo "$0 test ok (list with nonexistent parameter)!"
else
	echo "$0 test failed (list with nonexistent parameter)!"
	exit 1
fi

exit 0
