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

printf "list\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "list\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == 'dedb5c57680397dc993dd7b9c41240ed408e0e5c64aa85816eb8dd6ca17df2ee' ];then
	echo "$0 test ok (list)!"
else
	echo "$0 test failed (list)!"
	exit 1
fi

printf "list testchain\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "list testchain\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == 'd934aedfbf9f4c0f86828ad588b097b5d69ec9cebad7b11f7548af4452472cb1' ];then
	echo "$0 test ok (list with parameter)!"
else
	echo "$0 test failed (list with parameter)!"
	exit 1
fi

printf "list nonexistent\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "list nonexistent\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == '392f174e0d24350740598abeb595a3f6e4cfcd57c8ca6c9b1cc34c4e08b02ac0' ];then
	echo "$0 test ok (list with nonexistent parameter)!"
else
	echo "$0 test failed (list with nonexistent parameter)!"
	exit 1
fi

exit 0
