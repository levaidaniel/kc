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

SHA1=$(printf "c testchain\nlist\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^(default|testchain)% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "${SHA1}" = '58a77a2e6c812d38aaa2c35d9248739282d3b551' ];then
	echo "$0 test ok (change chain)!"
else
	echo "$0 test failed (change chain)!"
	exit 1
fi
printf "c 0\n" |./kc -b -k regress/test -p regress/testpass


printf "cnew 10\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA1=$(printf "c 10\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "${SHA1}" = '7972d6683794c10f1c092273fee326a19d78cbff' ];then
	echo "$0 test ok (change chain #2)!"
else
	echo "$0 test failed (change chain #2)!"
	exit 1
fi

SHA1=$(printf "cc 10\nlist\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^(default|10)% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "${SHA1}" = 'e5667529ede050810048938eb73abc278b86fa2d' ];then
	echo "$0 test ok (change chain (cc))!"
else
	echo "$0 test failed (change chain (cc))!"
	exit 1
fi

printf "ccdel 10\nyes\nwrite\n" |./kc -b -k regress/test -p regress/testpass


printf "c nonexistent\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "c nonexistent\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '7972d6683794c10f1c092273fee326a19d78cbff' ];then
	echo "$0 test ok (nonexistent chain)!"
else
	echo "$0 test failed (nonexistent chain)!"
	exit 1
fi

exit 0
