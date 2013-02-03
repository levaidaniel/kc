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
if [ "$SHA1" = '77fba7f4dbdf6ae7ea4bb2c694fb472506349524' ];then
	echo "$0 test ok (current chain)!"
else
	echo "$0 test failed (current chain)!"
	exit 1
fi

printf "!search y0\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "!search y0\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '1a109bc7e55dfe94540e4dc60a60f1e290a2a193' ];then
	echo "$0 test ok (current chain, inverse)!"
else
	echo "$0 test failed (current chain, inverse)!"
	exit 1
fi

printf "*search key\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "*search key\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '42c9ddafdbb323e86684162bf4d458dd96c118f9' ];then
	echo "$0 test ok (all chains)!"
else
	echo "$0 test failed (all chains)!"
	exit 1
fi

printf "!*search y1\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "!*search y1\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '78d1189766d1f5a0025c729bf068c0cf26c67469' ];then
	echo "$0 test ok (all chains, inverse)!"
else
	echo "$0 test failed (all chains, inverse)!"
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

exit 0
