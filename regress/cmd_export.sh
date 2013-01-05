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

printf "xport regress/test_export\n" |./kc -b -k regress/test -p regress/testpass

if [ ! -r "regress/test_export" ];then
	echo "$0 test failed (unreadable export file)!"
	exit 1
fi

SHA256=$($SHA256_BIN regress/test_export |cut -d' ' -f1)
if [ "$SHA256" = '0bca06de21e6dfa4b63361d57d7f002304b66341af9859d7bfea332c4ca061a4' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
