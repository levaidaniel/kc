#!/bin/sh -e

set -e


echo "test => $0"


if echo "" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} -P sha512 -P sha512;then
	echo "$0 test failed (invalid, multiple -P options)!"
	exit 1
else
	echo "$0 test ok (invalid, multiple -P options)!"
fi


if echo "" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} -P sha3 -K 15;then
	echo "$0 test failed (invalid, too small key length)!"
	exit 1
else
	echo "$0 test ok (invalid, too small key length)!"
fi


if echo "" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} -P sha3 -R 999;then
	echo "$0 test failed (invalid, too low number of KDF iterations)!"
	exit 1
else
	echo "$0 test ok (invalid, too low number of KDF iterations)!"
fi
