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

printf "/^edited_[abct]estke.[0-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "/^edited_[abct]estke.[0-9]$\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == 'eaadacae1c23f24c472ac7df97012c5818765c5cfca661623a24f8f475a57997' ];then
	echo "$0 test ok (current chain)!"
else
	echo "$0 test failed (current chain)!"
	exit 1
fi

printf "*/^edited_[abct]estke.[0-9]$\n" |./kc -b -k regress/test -p regress/testpass
SHA256=$(printf "*/^edited_[abct]estke.[0-9]$\n" |./kc -b -k regress/test -p regress/testpass |$SHA256_BIN |cut -d' ' -f1)
if [ "$SHA256" == '254f5d5a547877a6d8d88f383d9e3be3ad7071f904f977dda70acd477b562ee2' ];then
	echo "$0 test ok (all chains)!"
else
	echo "$0 test failed (all chains)!"
	exit 1
fi

exit 0
