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

# XXX
# This basically doesn't output anything with editline, beacuse the only notion
# of a successful keychain change is our prompt which displays the chain's name,
# and editline's prompt is not showing during batchmode...
SHA1=$(printf "c testchain\n" |./kc -b -k regress/test -p regress/testpass |$SHA1_BIN |cut -d' ' -f1)

if [ ${READLINE} ];then
	REFERENCE='7830303766409cbbd655e1c515e83653cf14e9a0'
else
	REFERENCE='e1e39a0d17ff84eaa4a4b90c5bd2a91c892954e6'
fi

if [ "${SHA1}" = "${REFERENCE}" ];then
	echo "$0 test ok (change chain)!"
else
	echo "$0 test failed (change chain)!"
	exit 1
fi

printf "c nonexistent\n" |./kc -b -k regress/test -p regress/testpass
SHA1=$(printf "c nonexistent\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '3cca28bb2a53a71c2a87c820da44d31f3fecf3b6' ];then
	echo "$0 test ok (nonexistent chain)!"
else
	echo "$0 test failed (nonexistent chain)!"
	exit 1
fi

exit 0
