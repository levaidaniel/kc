#!/bin/sh -e

set -e


typeset -i i=0
typeset -i loop=100


while [ $1 ];do
	case "$1" in
		'-l')
			shift
			loop=$1
		;;
		'-h')
			shift
			echo "$0 <-l loop count>"
			exit 0
		;;
	esac

	shift
done


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

if ./kc -v |grep -E -q -e '^Compiled with Readline(, RegExp)* support\.$';then
	export READLINE=readline
fi


# the sha256 sums for this to be enabled, are below and were generated for $loop=5000
typeset -i CHECK_DURING_MODIFY=0
[ $loop -eq 5000 ]  &&  CHECK_DURING_MODIFY=1
[ $CHECK_DURING_MODIFY -gt 0 ]  &&  echo "Checking db sum in between tests!"

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
SHA1_SUM_REFERENCE=$($SHA1_BIN regress/test |cut -d' ' -f1)

typeset -i offset=0


# new
i=0
while [ $i -lt ${loop} ];do
	printf "new $i\r" >/dev/stderr

	# random mode of command invocation for 'new'
	if [ $(( $RANDOM % 2 )) -eq 0 ];then
		printf "new\nnewkey$i\nnewval$i\n"
	else
		printf "new newkey$i\nnewval$i\n"
	fi

	# random writes
	if [ $(( $RANDOM % 2 )) -eq 0 ];then
		printf "write\n"
	fi

	if [ $i -eq $(( ${loop} - 1 )) ];then
		printf "write\n"
		break
	fi

	i=$(( $i + 1 ))
done |./kc -b -k regress/test -p regress/testpass >/dev/null
echo # new line

if [ ${CHECK_DURING_MODIFY} -gt 0 ];then
	SHA1=$(printf "list\n" |KC_DEBUG=yes ./kc -b -k regress/test -p regress/testpass |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/created="[0-9]\{1,\}"//' -e 's/modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
	if [ "$SHA1" == '2eeaa74bf795452dbfd532fe9499d22c761a9279' ];then
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
		printf "edit $i\nedited_newkey$(( $i - 2 ))\nedited_newval$(( $i - 2 ))\n"
	fi

	# random writes
	if [ $(( $RANDOM % 2 )) -eq 0 ];then
		printf "write\n"
	fi

	if [ $i -eq $(( ${loop} + ${offset} - 1 )) ];then
		printf "write\n"
		break
	fi

	i=$(( $i + 1 ))
done |./kc -b -k regress/test -p regress/testpass >/dev/null
echo # new line

if [ ${CHECK_DURING_MODIFY} -gt 0 ];then
	SHA1=$(printf "list\n" |KC_DEBUG=yes ./kc -b -k regress/test -p regress/testpass |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/created="[0-9]\{1,\}"//' -e 's/modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
	if [ "$SHA1" == '755c4e1e4d4951f58065188fcf1291dc25db8fcc' ];then
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

	# random writes
	if [ $(( $RANDOM % 2 )) -eq 0 ];then
		printf "write\n"
	fi

	if [ $i -eq ${offset} ];then
		printf "write\n"
		break
	fi

	i=$(( $i - 1 ))
done |./kc -b -k regress/test -p regress/testpass >/dev/null
echo # new line

SHA1=$($SHA1_BIN regress/test |cut -d' ' -f1)
if [ "$SHA1" = "$SHA1_SUM_REFERENCE" ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
