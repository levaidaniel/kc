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

if [ ${READLINE} ];then
	cmd="cren newchain\nrenamed_\nwrite\n"
else
	cmd="cren newchain\nrenamed_newchain\nwrite\n"
fi

printf "${cmd}" |./kc -b -k regress/test -p regress/testpass

SHA1=$($SHA1_BIN regress/test |cut -d' ' -f1)
if [ "$SHA1" = 'd8d394b445da5240483016662265638be3197688' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
