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

echo -n ${PASSWORD} > ${KC_PASSFILE}
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
q7U64xxnjspcdjl+4UfHIjDariqiIrNu955VD7HKLTWg6hr19RaQYN0Fe8CdVD7S
CxlyA2ieqsJNZiIxO4lFI6TCHm9gx1aKVle1fVUYf2fBxVD4289pDgGXabgwzRNC
xXkXcsWZvJO43FP+mLS2uatr0CiyoMdEOEMlnpoJ65apa37c4fgj/hpwRwzF/jxH
OB+BPSkFPNlmkwAHrHfSZRblZK7Gt0DbRhCts8UERI34/RHZ/rg7UmvFFVgY4YmE
zF5rO7N3t4nhCZbM6Lh75rjAHNznAILe+MT97FPZ5F3MA/nZ/rL8YS2eUAPc3A3E
pMBwq8WWNsYwavdfEvIGsb76TwyUTxVGoco0zctiVchXvQbyCyPHPnphS/HywO1F
k2ExWELRCf8Xop7eBmczfaps84WjuJxW4fSaEIpEwX0Yal4Fp4+ZCJ+2H1T7sNqv
az94glm3vINSpVXMNLfOD5gVV02l4UAns3Wo4c8XxbQG0JpDZCUjY6rbJbiUpn+I' > ${KC_DB}

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
