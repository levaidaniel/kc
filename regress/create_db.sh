#!/bin/sh -e

set -e


echo "test => $0"

if [ -z "${KC_DB}" ]  ||  [ -z ${KC_PASSFILE} ];then
	echo "No database or password file specified!";
	exit 1;
fi

rm -f ${KC_DB} ${KC_PASSFILE}

PASSWORD='qNYHMvXunofKXY7NSBCmDa2T4Av8R2rJWk15ADvca0IHkFqpiqriPnjtQHzWeZBq2MCk3SpoSc6aer5VO33RRC0aM85mxtid50gUUwRT0OhsfKxpTTBdr1hxwQhklpQZj5F28GbDYE5OPWWVKxzmhsbpdt1cfmJw8vG3vn8j3KqcljIn7UdTyJl8yGjtnmWR6wE3G4OW2mEZE8ruX2GnYIHBNwRKc71AakejERXObtdIFQfjxY4V6nyTWPkcPRLT'

# create a random database
if printf "${PASSWORD}\n${PASSWORD}\nwrite\n" |${KC_RUN} -b -k ${KC_DB};then
	echo "$0 test ok (create random db)!"
else
	echo "$0 test failed (create random db)!"
	exit 1
fi

echo ${PASSWORD} > ${KC_PASSFILE}
if echo "" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE};then
	echo "$0 test ok (open random db)!"
else
	echo "$0 test failed (open random db)!"
	exit 1
fi

rm -f ${KC_DB}


# create our sample database
echo '5216cd24bfc3cfa5e6eed9ba5df68ee829e1034f9c1491c440d242c7e02a65278463d5e321569e561fb4a8adbef243d88612aa19cb79fe63204283488cb1ae84
14ee9bd9d53fec4e18d94b1a037da791521e141efe967f110be78ba3c263ea7d4c2da2b2a5d667d27d8c73aadfdb21e7e706208c981091b55bba860085f2410f
XRrihuQniAtbSIn+3bAekEuYXbrVUFy+BbpzAWxqNnO35NWzsKFKL7pRm2eM1D8E
q7U64xxnjspcdjl+4UfHIhmsyvXUL4nkm1T148xphPfM9t2/qeKL/ds4X4hn10Uk
vge3yEHEk59sE4UAlYmSyQicg6AMLhDSLWyx+KnMsvynO4mEADUSZyP0LKLPT9qI
Igc2uxHXVeuWp9BC/DjnZ7F3Mt6Xm8KYI8wrkc9GQrGpQcd4qFLO1UyX0fWsAltA
cy752aGVJMpWpk4ZIzcuEhGnWXxOZ0mNqYIz4a4NYkjGK7+Z3lDb+yaCqAUZxO1M
8fZ9TDMqnizGLqKnO67+GDL9w7Ncyh9DZynksGqMEUHrKRBK8V18pmOydqt5hv3g
oSGaIBDUDKRv6M0fHhmAr6/J3ygTwZRT30M1K07CCxzD3Q9EDhVNPN0RS6MLuvji
m1xYj7fmah14BRissoEdlw4cg1RETUDVD0CCRdHP7gE=' > ${KC_DB}

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
