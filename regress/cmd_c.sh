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

# XXX
# This basically doesn't output anything with editline, beacuse the only notion
# of a successful keychain change is our prompt which displays the chain's name,
# and editline's prompt is not showing during batchmode...
SHA256=$(printf "c testchain\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)

if [ ${READLINE} ];then
	REFERENCE='0fe30cb29f4485667bd5b07fbd3470af9488876da0f2c6303a61e7c425744221'
else
	REFERENCE='ab65722c447698811e5c11fb49fa9dd277fbd7cf367f871c8d94c7e4c6f1825b'
fi

if [ "${SHA256}" == "${REFERENCE}" ];then
	echo "$0 test ok (change chain)!"
else
	echo "$0 test failed (change chain)!"
	exit 1
fi

printf "c nonexistent\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "c nonexistent\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == '3bb0673128fe11dd7add24efab95349b07c45cea2971b8691e85c69427a6d297' ];then
	echo "$0 test ok (nonexistent chain)!"
else
	echo "$0 test failed (nonexistent chain)!"
	exit 1
fi

exit 0
