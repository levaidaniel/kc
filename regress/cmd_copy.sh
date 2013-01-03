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

printf "copy 0 testchain\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)
if [ "$SHA256" == '949d3896aea10ecc3b4c472aaf70f7f8d09f44a2d3812dc04b191a498be98abb' ];then
	echo "$0 test ok (copy)!"
else
	echo "$0 test failed (copy)!"
	exit 1
fi

printf "c testchain\nmove 0 default\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)
if [ "$SHA256" == '0a9e9fec74bf2b80a1fc136c3a497c12e0f47bde60db45cb19b0ea0470dfcfc1' ];then
	echo "$0 test ok (move)!"
else
	echo "$0 test failed (move)!"
	exit 1
fi

exit 0
