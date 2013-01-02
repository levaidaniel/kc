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

printf "write\n" |./kc -b -k regress/test -p regress/testpass

SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)
if [ "$SHA256" == 'b083b796a99a3acb57e0a36c581b689d8e441374338a7fccb12970a8306b5c97' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
