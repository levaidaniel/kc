#!/bin/sh -e

set -e


echo "test => $0"

PASSWORD=$(< ${KC_PASSFILE})
NEWPASSWORD='123abc321ABC'

if printf "passwd\n${NEWPASSWORD}\n${NEWPASSWORD}\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE};then
	echo "$0 test ok (passwd)!"
else
	echo "$0 test failed (passwd)!"
	exit 1
fi

echo "${NEWPASSWORD}" > ${KC_PASSFILE}

if printf "passwd\n${PASSWORD}\n${PASSWORD}\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE};then
	echo "$0 test ok (orig passwd)!"
else
	echo "$0 test failed (orig passwd)!"
	exit 1
fi

echo "${PASSWORD}" > ${KC_PASSFILE}

if printf "passwd bcrypt\n${PASSWORD}\n${PASSWORD}\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE};then
	echo "$0 test ok (passwd bcrypt #1)!"
else
	echo "$0 test failed (passwd bcrypt #1)!"
	exit 1
fi

if printf "passwd sha512\n${PASSWORD}\n${PASSWORD}\n" |./kc -b -k ${KC_DB} -P bcrypt -p ${KC_PASSFILE};then
	echo "$0 test ok (passwd bcrypt #2)!"
else
	echo "$0 test failed (passwd bcrypt #2)!"
	exit 1
fi

if echo "" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE};then
	echo "$0 test ok (reopen)!"
else
	echo "$0 test failed (reopen)!"
	exit 1
fi

exit 0
