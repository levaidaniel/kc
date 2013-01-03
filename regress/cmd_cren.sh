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

if [ ${READLINE} ];then
	cmd="cren newchain\nrenamed_\nwrite\n"
else
	cmd="cren newchain\nrenamed_newchain\nwrite\n"
fi

printf "${cmd}" |./kc -b -k regress/test -p regress/testpass

SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)
if [ "$SHA256" == '86e1c5060f3042419bdaaa5b6b8e8d257eaf693dad935c72003424024e28bc44' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
