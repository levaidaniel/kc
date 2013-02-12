#!/bin/sh -e

set -e


echo "test => $0"

if ! ./kc -v |grep -E -q -e '^Compiled with (Readline|Editline), PCRE support\.$';then
	echo "$0 - Not running without PCRE support compiled in!"
	exit 0
fi

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

printf "/ ^d.*.[abck]ey[0-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "/ ^d.*.[abck]ey[0-9]+$\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '646c97c66d3c5f0810adfabe230ba164a182825e' ];then
	echo "$0 test ok (current chain)!"
else
	echo "$0 test failed (current chain)!"
	exit 1
fi

printf "/i ^d.*.[ABCK]EY[0-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "/i ^d.*.[ABCK]EY[0-9]+$\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '646c97c66d3c5f0810adfabe230ba164a182825e' ];then
	echo "$0 test ok (current chain, ignore case)!"
else
	echo "$0 test failed (current chain, ignore case)!"
	exit 1
fi

printf "/! ^d.*.[abck]ey[1-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "/! ^d.*.[abck]ey[1-9]+$\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'fdaeba482a0e65568d029c079bf6999e9d1224b9' ];then
	echo "$0 test ok (current chain, inverse)!"
else
	echo "$0 test failed (current chain, inverse)!"
	exit 1
fi

printf "/* ^(default)*[abck]ey(test)*[0-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "/* ^(default)*[abck]ey(test)*[0-9]$\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'dc238e4c5b82d4c0d03eb301297285eb4bdf2f66' ];then
	echo "$0 test ok (all chains)!"
else
	echo "$0 test failed (all chains)!"
	exit 1
fi

printf "/!* ^(default)*[abck]ey(test)*[1-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "/!* ^(default)*[abck]ey(test)*[1-9]$\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'cb17e2cbab306a1f971d16105441a9c7cf6d5d61' ];then
	echo "$0 test ok (all chains, inverse)!"
else
	echo "$0 test failed (all chains, inverse)!"
	exit 1
fi

printf "/!*i ^(default)*[ABCK]EY(tESt)*[1-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "/!*i ^(default)*[ABCK]EY(tESt)*[1-9]$\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'cb17e2cbab306a1f971d16105441a9c7cf6d5d61' ];then
	echo "$0 test ok (all chains, inverse, ignore case)!"
else
	echo "$0 test failed (all chains, inverse, ignore case)!"
	exit 1
fi

printf "/ nonexistent\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "/ nonexistent\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'bca9dfb5d8e5d96ece5d1847c95972736c1d662d' ];then
	echo "$0 test ok (nonexistent)!"
else
	echo "$0 test failed (nonexistent)!"
	exit 1
fi

printf "c/ chain\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "c/ chain\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '1d8dca2a0bbb6ac6e4260f2734e5801a42e00e32' ];then
	echo "$0 test ok (c/)!"
else
	echo "$0 test failed (c/)!"
	exit 1
fi

printf "c/i chain\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "c/i chain\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '1d8dca2a0bbb6ac6e4260f2734e5801a42e00e32' ];then
	echo "$0 test ok (c/i, ignore case)!"
else
	echo "$0 test failed (c/i, ignore case)!"
	exit 1
fi

printf "c/! chain\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "c/! chain\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'ef7cac9a6237d46aacb5b1990945004a1cfbdf22' ];then
	echo "$0 test ok (c/, inverse)!"
else
	echo "$0 test failed (c/, inverse)!"
	exit 1
fi

exit 0
