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

printf "list\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE}
SHA1=$(printf "list\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '040b16edbb11c9e9e3da9c09389000a34d473a6a' ];then
	echo "$0 test ok (list)!"
else
	echo "$0 test failed (list)!"
	exit 1
fi

exit 0
