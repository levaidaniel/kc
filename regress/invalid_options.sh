#!/bin/sh -e

set -e


echo "test => $0"


if echo "" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} -P sha512 -P sha512;then
	echo "$0 test failed (invalid, multiple -P options)!"
	exit 1
else
	echo "$0 test ok (invalid, multiple -P options)!"
fi
