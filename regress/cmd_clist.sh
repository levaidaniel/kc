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

printf "clist\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >'
SHA1=$(printf "clist\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |$SHA1_BIN |cut -d' ' -f1)

if [ "$SHA1" = '921b21c0ffcaf71e2843d4acaff89602d63819cd' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
