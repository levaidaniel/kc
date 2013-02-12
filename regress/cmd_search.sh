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

printf "search key\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "search key\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '646c97c66d3c5f0810adfabe230ba164a182825e' ];then
	echo "$0 test ok (current chain)!"
else
	echo "$0 test failed (current chain)!"
	exit 1
fi

printf "searchi KEY\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "searchi KEY\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '646c97c66d3c5f0810adfabe230ba164a182825e' ];then
	echo "$0 test ok (current chain, ignore case)!"
else
	echo "$0 test failed (current chain, ignore case)!"
	exit 1
fi

printf "search! y0\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "search! y0\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'f94234af3345169c5c28309499883b8a127b7d04' ];then
	echo "$0 test ok (current chain, inverse)!"
else
	echo "$0 test failed (current chain, inverse)!"
	exit 1
fi

printf "search* key\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "search* key\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'dc238e4c5b82d4c0d03eb301297285eb4bdf2f66' ];then
	echo "$0 test ok (all chains)!"
else
	echo "$0 test failed (all chains)!"
	exit 1
fi

printf "search!* y1\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "search!* y1\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '9167daa2b4afcc2727364f6f77cf2a1a73cd6954' ];then
	echo "$0 test ok (all chains, inverse)!"
else
	echo "$0 test failed (all chains, inverse)!"
	exit 1
fi

printf "search!*i Y1\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "search!*i Y1\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '9167daa2b4afcc2727364f6f77cf2a1a73cd6954' ];then
	echo "$0 test ok (all chains, inverse, ignore case)!"
else
	echo "$0 test failed (all chains, inverse, ignore case)!"
	exit 1
fi

printf "search nonexistent\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "search nonexistent\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'bca9dfb5d8e5d96ece5d1847c95972736c1d662d' ];then
	echo "$0 test ok (nonexistent)!"
else
	echo "$0 test failed (nonexistent)!"
	exit 1
fi

printf "csearch chain\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "csearch chain\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '1d8dca2a0bbb6ac6e4260f2734e5801a42e00e32' ];then
	echo "$0 test ok (csearch)!"
else
	echo "$0 test failed (csearch)!"
	exit 1
fi

printf "csearchi CHAIN\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "csearchi CHAIN\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '1d8dca2a0bbb6ac6e4260f2734e5801a42e00e32' ];then
	echo "$0 test ok (csearchi, ignore case)!"
else
	echo "$0 test failed (csearchi, ignore case)!"
	exit 1
fi

printf "csearch! chain\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "csearch! chain\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'ef7cac9a6237d46aacb5b1990945004a1cfbdf22' ];then
	echo "$0 test ok (csearch, inverse)!"
else
	echo "$0 test failed (csearch, inverse)!"
	exit 1
fi

exit 0
