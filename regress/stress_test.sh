#!/bin/sh -e

set -e


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

if ./kc -v |grep -E -q -e '^Compiled with Readline(, PCRE)* support\.$';then
	export READLINE=readline
fi


# the sha256 sums for this to be enabled, are below and were generated for $loop=5000
typeset -i CHECK_DURING_MODIFY=0
[ $loop -eq 5000 ]  &&  CHECK_DURING_MODIFY=1
[ $CHECK_DURING_MODIFY -gt 0 ]  &&  echo "Checking db sum in between tests!"


sh regress/create_db.sh


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
	SHA1=$(printf "list\n" |KC_DEBUG=yes ./kc -b -k regress/test -p regress/testpass |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' -e 's/ description=".*"//' |$SHA1_BIN |cut -d' ' -f1)
	if [ "$SHA1" == '796ef067438935598affa049915a6b1438abb981' ];then
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
	SHA1=$(printf "list\n" |KC_DEBUG=yes ./kc -b -k regress/test -p regress/testpass |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' -e 's/ description=".*"//' |$SHA1_BIN |cut -d' ' -f1)
	if [ "$SHA1" == '75e950b88253fd82161097aa11133aff50df8dd1' ];then
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

SHA1=$(printf "list\n" |KC_DEBUG=yes ./kc -b -k regress/test -p regress/testpass |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' -e 's/ description=".*"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'f7c121366b5a79c8865a9df29bf7143f3b65b4ec' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
