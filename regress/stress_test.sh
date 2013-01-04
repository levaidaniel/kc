#!/bin/sh -e

set -e


[ "$1" == 'readline' ]  &&  export READLINE=readline

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

typeset -i CHECK_DURING_MODIFY=0

typeset -i i=0
typeset -i loop=5000
typeset -i offset=0

# this is our sample database from regress/create_db.sh
echo 'abc123ABC321' > regress/testpass
echo 'w{CQ5/H^DgPv[$[qHc`*oc*SZH*|G`*.nw33FG5BaciY6pc4aIabq7ORQW6kyZH8VNIEog/32cwnCDsQ0iy+4RU+Z2Y+W9Wx
Zkfqnpdr1boE5lEW869+AmKMxL9OhKcagB6NNiM1SxtSNf7mMtnOI8YYIh/15tfT
7V8SIQYFJmeI8tIpm8NvBsP8ELUXSnlEZVqbvXgC9SRYP6iwUUq8g/N6MMzG+EoS
Y0W/LGOtEzhPBXAwcVp2+akFZ9DR34F2PM/IuauvdW+UZl1I2ObWNPFzde62nMLU
kloxN6u+ReX+Hi37LsWQuL4yzYkl+pN6DH4wh+vamRc6wYxDImVMi5b9K90/VyDB
KrbeoLrGsNAFJ1paFuXinmwn2hVS11+5ayEfadHgpsi0cm4Ze9xoroM0eX+Vj7LT
FIOgRFau1xvBM2kauEalqRwYKTjOmTdv41iEKK+FaDhhYl2OId5vbHJSp52OF51s
iRaLY/GO3OsRjdwAOVC8byYKJMWmeAtqoEbDVYfSANuSmDp85XJSwrQ0HvA+pX9o
O8zhUz1Cy+avitWpRn5RsQ==' > regress/test
SHA256_SUM_REFERENCE=$($SHA256_BIN regress/test |cut -d' ' -f1)



# new
i=0
while [ $i -lt ${loop} ];do
	printf "new $i\r" >/dev/stderr

	printf "new newkey$i\nnewval$i\n"

	if [ $i -eq $(( ${loop} - 1 )) ];then
		printf "write\n"
		break
	fi

	i=$(( $i + 1 ))
done |./kc -b -k regress/test -p regress/testpass >/dev/null
echo # new line

if [ ${CHECK_DURING_MODIFY} -gt 0 ];then
	SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)
	if [ "$SHA256" == 'd333279529484e9d2bfb40c11d013191da7970d19e716742f2ee6a27541649dd' ];then
		echo "$0 test ok (#${loop} new entry)!"
	else
		echo "$0 test failed (#${loop} new entry)!"
		exit 1
	fi
fi



# edit
offset=2
i=$(( 0 + ${offset} ))
while [ $i -lt $(( ${loop} + ${offset} )) ];do
	printf "edit $(( $i - ${offset} ))\r" >/dev/stderr

	if [ ${READLINE} ];then
		printf "edit $i\nedited_\nedited_\n"
	else
		printf "edit $i\nedited_newkey\nedited_newval\n"
	fi

	if [ $i -eq $(( ${loop} + ${offset} - 1 )) ];then
		printf "write\n"
		break
	fi

	i=$(( $i + 1 ))
done |./kc -b -k regress/test -p regress/testpass >/dev/null
echo # new line

if [ ${CHECK_DURING_MODIFY} -gt 0 ];then
	SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)
	if [ "$SHA256" == 'fbfb29703234f8605ceadb7e978eebc93003c4fab6b0836cad9e6aa5fb7c3998' ];then
		echo "$0 test ok (#${loop} entry edit)!"
	else
		echo "$0 test failed (#${loop} entry edit)!"
		exit 1
	fi
fi



# del
offset=2
i=$(( ${loop} + 1 ))
typeset -i todel=0
while [ $i -ge ${offset} ];do
	todel=0
	while [ ${todel} -lt ${offset} ]  ||  [ ${todel} -gt $i ];do
		todel=$RANDOM
	done
	printf "           \r" >/dev/stderr
	printf "del $i\r" >/dev/stderr

	printf "del ${todel}\nyes\n"

	# random writes, when the index is even
	if [ $(( $i % 2 )) -eq 0 ];then
		printf "write\n"
	fi

	if [ $i -eq ${offset} ];then
		printf "write\n"
		break
	fi

	i=$(( $i - 1 ))
done |./kc -b -k regress/test -p regress/testpass >/dev/null
echo # new line

SHA256=$($SHA256_BIN regress/test |cut -d' ' -f1)
if [ "$SHA256" == "$SHA256_SUM_REFERENCE" ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
