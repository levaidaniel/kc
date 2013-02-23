#!/bin/sh -e

set -e


echo "test => $0"

PASSWORD='123abc321ABC'

if printf "passwd\n${PASSWORD}\n${PASSWORD}\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE};then
	echo "$0 test ok (passwd)!"
else
	echo "$0 test failed (passwd)!"
	exit 1
fi

echo "${PASSWORD}" > ${KC_PASSFILE}

if echo "" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE};then
	echo "$0 test ok (reopen)!"
else
	echo "$0 test failed (reopen)!"
	exit 1
fi

exit 0
