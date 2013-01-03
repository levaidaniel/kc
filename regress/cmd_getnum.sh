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

printf "0\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "0\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == '0fb5e29935c233aea31ed67fc62703b5673986d6083516e8a5a11edb0030ef42' ];then
	echo "$0 test ok (existing)!"
else
	echo "$0 test failed (existing)!"
	exit 1
fi

printf "2\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "2\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == 'd90439f5631422f4a557ea7b0ef54faf12d8db17efb85db6aa80652a55ad3654' ];then
	echo "$0 test ok (nonexisting)!"
else
	echo "$0 test failed (nonexisting)!"
	exit 1
fi

exit 0
