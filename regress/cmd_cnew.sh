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

printf "cnew testchain\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)

if [ "$SHA256" == 'e22f33990a2a576de6402650d42ea02f60fe31458cdf08c226ed0d30e10f1156' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
