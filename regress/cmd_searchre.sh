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
if [ "$SHA1" = '77fba7f4dbdf6ae7ea4bb2c694fb472506349524' ];then
	echo "$0 test ok (current chain)!"
else
	echo "$0 test failed (current chain)!"
	exit 1
fi

printf "!/^d.*.[abck]ey[1-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "!/^d.*.[abck]ey[1-9]+$\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'ca67c5eaf77d132c1269fa8be8b8e1243ff4a345' ];then
	echo "$0 test ok (current chain, inverse)!"
else
	echo "$0 test failed (current chain, inverse)!"
	exit 1
fi

printf "*/^(default)*[abck]ey(test)*[0-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "*/^(default)*[abck]ey(test)*[0-9]$\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '42c9ddafdbb323e86684162bf4d458dd96c118f9' ];then
	echo "$0 test ok (all chains)!"
else
	echo "$0 test failed (all chains)!"
	exit 1
fi

printf "!*/^(default)*[abck]ey(test)*[1-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "!*/^(default)*[abck]ey(test)*[1-9]$\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'd88e597028577aa8062f464d741503abea60d9dd' ];then
	echo "$0 test ok (all chains, inverse)!"
else
	echo "$0 test failed (all chains, inverse)!"
	exit 1
fi

printf "/nonexistent\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "/nonexistent\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'bca9dfb5d8e5d96ece5d1847c95972736c1d662d' ];then
	echo "$0 test ok (nonexistent)!"
else
	echo "$0 test failed (nonexistent)!"
	exit 1
fi

exit 0
