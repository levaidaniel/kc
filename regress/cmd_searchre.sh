#!/bin/sh -e

set -e


echo "test => $0"

case "$(uname -s)" in
	Linux)
		SHA1_BIN=$(which sha1sum)
	;;
	*BSD)
		SHA1_BIN="$(which sha1) -r"
	;;
	*)
		echo "unknown system."
		exit 1
	;;
esac

printf "/^d.*.[abck]ey[0-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "/^d.*.[abck]ey[0-9]+$\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '646c97c66d3c5f0810adfabe230ba164a182825e' ];then
	echo "$0 test ok (current chain)!"
else
	echo "$0 test failed (current chain)!"
	exit 1
fi

printf "*/^(default)*[abck]ey(test)*[0-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "*/^(default)*[abck]ey(test)*[0-9]$\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'dc238e4c5b82d4c0d03eb301297285eb4bdf2f66' ];then
	echo "$0 test ok (all chains)!"
else
	echo "$0 test failed (all chains)!"
	exit 1
fi

printf "/nonexistent\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "/nonexistent\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'c5a81f8b956e8aa4cc066a4232be5db06b7f9572' ];then
	echo "$0 test ok (nonexistent)!"
else
	echo "$0 test failed (nonexistent)!"
	exit 1
fi

exit 0
