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

printf "/^d.*.[abck]ey[0-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "/^d.*.[abck]ey[0-9]+$\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == '4f6ab60a1abd21a8d0c16a0fef60a94b95b549e49d5cd716e9f3bc5f2d1f9991' ];then
	echo "$0 test ok (current chain)!"
else
	echo "$0 test failed (current chain)!"
	exit 1
fi

printf "*/^(default)*[abck]ey(test)*[0-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "*/^(default)*[abck]ey(test)*[0-9]$\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == 'ae8c6394be737e3d7ba27ac2e16213591126dc33350bc96d22a7a89ea57d438c' ];then
	echo "$0 test ok (all chains)!"
else
	echo "$0 test failed (all chains)!"
	exit 1
fi

printf "/nonexistent\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "/nonexistent\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == 'b1ab5f2f45667fd6a6f82bbaf5639168d16f14e385f0fbe1bcbc7e0262ba9819' ];then
	echo "$0 test ok (nonexistent)!"
else
	echo "$0 test failed (nonexistent)!"
	exit 1
fi

exit 0
