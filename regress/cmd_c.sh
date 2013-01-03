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

printf "c testchain\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "c testchain\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)

if [ "$SHA256" == '0fe30cb29f4485667bd5b07fbd3470af9488876da0f2c6303a61e7c425744221' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
