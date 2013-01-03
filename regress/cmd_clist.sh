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

printf "clist\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default%'
SHA256=$(printf "clist\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default%' |$SHA256_BIN |cut -d' ' -f1)

if [ "$SHA256" == '33cab88cabb11191d6b4f13820b591fd8102fea564985666ef7d3a869f692d32' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
