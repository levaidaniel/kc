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

printf "clist\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "clist\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)

if [ "$SHA256" == 'f0f5a4a4f1a26d9710085a0a3b2a9be501200d5ba4cb7bbde92a757e57e259d7' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
