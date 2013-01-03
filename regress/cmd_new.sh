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

printf "new testkey0\ntestval0\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)
if [ "$SHA256" == '052de2ee5d7598904a3c1e3ecf920cd3335d4712939e8f3de50708c27bd0be01' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
