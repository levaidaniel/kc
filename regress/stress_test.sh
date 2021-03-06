#!/bin/sh -e

set -e

ulimit -c unlimited


typeset -i i=0
typeset -i loop=5000


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
	Linux|CYGWIN*)
		SHA1_BIN=$(which sha1sum)
		RANDOM_DEV="/dev/urandom"
	;;
	*BSD)
		SHA1_BIN="$(which sha1) -r"
		RANDOM_DEV="/dev/random"
	;;
	*)
		echo "unknown system."
		exit 1
	;;
esac
export SHA1_BIN
export RANDOM_DEV


export KC_RUN=${KC_RUN:-'./kc'}


if ${KC_RUN} -v |grep -E -q -e '^Compiled with Readline(, PCRE)*(, scrypt)*(, YubiKey)* support\.$';then
	export READLINE=readline
fi


# the sha256 sums for this to be enabled, are below and were generated for $loop=5000
typeset -i CHECK_DURING_MODIFY=0
[ $loop -eq 5000 ]  &&  CHECK_DURING_MODIFY=1
[ $CHECK_DURING_MODIFY -gt 0 ]  &&  echo "Checking db sum in between tests!"


export KC_DB='regress/test.kcd'
export KC_PASSFILE='regress/testpass'
sh regress/create_db.sh


RANDOM=$$
typeset -i offset=0

# new
i=0
while [ $i -lt ${loop} ];do
	printf "new $i\r" >&2

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
done |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} >/dev/null
echo # new line

if [ ${CHECK_DURING_MODIFY} -gt 0 ];then
	SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
	if [ "$SHA1" == '828da68620831478eadf432fda61dffde06f6f0f' ];then
		echo "$0 test ok (#${loop} new entry)!"
	else
		echo "$0 test failed (#${loop} new entry)!"
		exit 1
	fi
fi



# insert
i=0
while [ $i -lt $(( ${loop} + 2 )) ];do
	printf "insert #$i\r" >&2

	printf "insert 0 $(( ${loop} + 1 ))\n"

	# random writes
	if [ $(( $RANDOM % 2 )) -eq 0 ];then
		printf "write\n"
	fi

	if [ $i -eq $(( ${loop} + 1 )) ];then
		printf "write\n"
		break
	fi

	i=$(( $i + 1 ))
done |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} >/dev/null
echo # new line

if [ ${CHECK_DURING_MODIFY} -gt 0 ];then
	SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
	if [ "$SHA1" == '828da68620831478eadf432fda61dffde06f6f0f' ];then
		echo "$0 test ok (#${loop} inserts)!"
	else
		echo "$0 test failed (#${loop} inserts)!"
		exit 1
	fi
fi



# edit
offset=2
i=$(( 0 + ${offset} ))
while [ $i -lt $(( ${loop} + ${offset} )) ];do
	printf "edit $(( $i - ${offset} ))\r" >&2

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
done |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} >/dev/null
echo # new line

if [ ${CHECK_DURING_MODIFY} -gt 0 ];then
	SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
	if [ "$SHA1" == '02efaae2e73b313afd254c4d4464b84e8e63849b' ];then
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
	printf "           \r" >&2
	printf "del $i\r" >&2

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
done |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} >/dev/null
echo # new line

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'ba72861439b641d0f7392162bb65f673765b0b3f' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
