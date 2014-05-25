#!/bin/sh -e

set -e


echo "test => $0"

if echo exit |${KC_RUN} -b -c testchain -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -q -e "^Starting with keychain 'testchain'.\$";then
	echo "$0 test ok (change chain with -c option)!"
else
	echo "$0 test failed (change chain with -c option)!"
	exit 1
fi

printf "cnew 10\ndescription\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} >/dev/null

if echo exit |${KC_RUN} -b -c 10 -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -q -e "^Keychain '10' not found, starting with the first one!\$";then
	echo "$0 test ok (change chain with -c option #2)!"
else
	echo "$0 test failed (change chain with -c option #2)!"
	exit 1
fi

if echo exit |${KC_RUN} -b -C 10 -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -q -e "^Starting with keychain '10'.\$";then
	echo "$0 test ok (change chain with -C option)!"
else
	echo "$0 test failed (change chain with -C option)!"
	exit 1
fi

printf "ccdel 10\nyes\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} >/dev/null

exit 0
