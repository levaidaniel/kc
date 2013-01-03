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

printf "cren testchain\nrenamed_\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)
if [ "$SHA256" == '8667dd38f996e1cc85158a03965e0cccae89d49970a032223688649870aa7798' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
