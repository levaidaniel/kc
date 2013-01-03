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
	cmd="edit 2\nedited_\nedited_\nwrite\n"
else
	cmd="edit 2\nedited_newkey\nedited_newval\nwrite\n"
fi

printf "${cmd}" |./kc -b -k regress/test -p regress/testpass

SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)
if [ "$SHA256" == 'f62a449b622fae5c750dd0412bb1bbe8c24af8dc0142495275a98ac201178908' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
