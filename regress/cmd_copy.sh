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

printf "copy 0 emptychain\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)
if [ "$SHA256" == 'bdd43b973530c8f7fdd5ce81bbfc4bc03c3f224eb7514dadf2c8dd8c659aeb24' ];then
	echo "$0 test ok (copy)!"
else
	echo "$0 test failed (copy)!"
	exit 1
fi

printf "c emptychain\nmove 0 default\nwrite\n" |./kc -b -k regress/test -p regress/testpass

SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)
if [ "$SHA256" == '03ba9af771ab3090cd3662063b008a2cfac3e94fd368f411eb9405f3d68e1d78' ];then
	echo "$0 test ok (move)!"
else
	echo "$0 test failed (move)!"
	exit 1
fi

exit 0
