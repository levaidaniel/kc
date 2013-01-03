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

printf "new newkey\nnewval\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)
if [ "$SHA256" == '2da10429426799e37adcf8947bc61ae17f479c1b76d677ebfe74159285e943c8' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
