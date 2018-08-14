#!/bin/sh -e

set -e


echo "test => $0"

if [ -z "${KC_DB}" ]  ||  [ -z ${KC_PASSFILE} ];then
	echo "No database or password file specified!";
	exit 1;
fi

rm -f ${KC_DB} ${KC_PASSFILE}

# create a new database with a random password
dd if=${RANDOM_DEV} of=${KC_PASSFILE} bs=1024 count=1
if echo 'write' |${KC_RUN} -b -p ${KC_PASSFILE} -k ${KC_DB};then
	echo "$0 test ok (create db with random password)!"
else
	echo "$0 test failed (create db with random password)!"
	exit 1
fi
rm -f ${KC_DB}

# create a new database with a pre-defined password
PASSWORD='qNYHMvXunofKXY7NSBCmDa2T4Av8R2rJWk15ADvca0IHkFqpiqriPnjtQHzWeZBq2MCk3SpoSc6aer5VO33RRC0aM85mxtid50gUUwRT0OhsfKxpTTBdr1hxwQhklpQZj5F28GbDYE5OPWWVKxzmhsbpdt1cfmJw8vG3vn8j3KqcljIn7UdTyJl8yGjtnmWR6wE3G4OW2mEZE8ruX2GnYIHBNwRKc71AakejERXObtdIFQfjxY4V6nyTWPkcPRLT'
if printf "${PASSWORD}\n${PASSWORD}\nwrite\n" |${KC_RUN} -b -k ${KC_DB};then
	echo "$0 test ok (create empty db)!"
else
	echo "$0 test failed (create empty db)!"
	exit 1
fi

echo -n ${PASSWORD} > ${KC_PASSFILE}
if echo "" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE};then
	echo "$0 test ok (open empty db)!"
else
	echo "$0 test failed (open empty db)!"
	exit 1
fi

rm -f ${KC_DB}


# create our sample database
if printf "importxml -k regress/sample.xml\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE};then
	echo "$0 test ok (import sample db)!"
else
	echo "$0 test failed (import sample db)!"
fi

if [ ! -r "${KC_DB}" ];then
	echo "$0 test failed (read sample db)!"
	exit 1
fi

if printf "${PASSWORD}" |${KC_RUN} -b -k ${KC_DB};then
	echo "$0 test ok (create and open sample db)!"
else
	echo "$0 test failed (create and open sample db)!"
	exit 1
fi

exit 0
