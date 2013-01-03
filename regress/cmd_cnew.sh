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

printf "cnew newchain\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)

if [ "$SHA256" == 'a2fd6b48e9019656cac498f1adc523a920a1e385395b5397a8aa3578fa8ee8b7' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
