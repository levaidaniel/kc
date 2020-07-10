#!/bin/sh

ulimit -c unlimited


if [ $(basename $(pwd))  = 'regress' ];then
	echo "please run this script directly from the source directory."
	exit 1
fi

export KC_RUN=${KC_RUN:-'./kc'}


# Figure out the compiled-in features
VERSION=$( ${KC_RUN} -v |grep -E -e '^Compiled with' )

if echo "${VERSION}" |grep -F -q -e 'Readline';then
	export READLINE=readline
fi
if echo "${VERSION}" |grep -F -q -e 'SCRYPT';then
	export SCRYPT=scrypt
fi


export KC_DB='regress/test.kcd'
export KC_PASSFILE='regress/testpass'

trap '
if [ $RETVAL -eq 0 ];then
	if [ $SKIPPED -eq 0 ];then
		printf "\nAll ($COUNTER) tests were ok! :)\n"
	else
		printf "\n$(($COUNTER - $SKIPPED)) tests were ok, $SKIPPED skipped!\n"
	fi
	echo "Took about $((SECONDS - START)) seconds"
else
	printf "\nTest #$COUNTER failed! :(\n" 1>&2
fi
' EXIT

trap '
echo "Aborted."
' TERM KILL INT

export SHA1_INVALID_INDEX='812e96292afbdf1b0cebb40a7db6a7ffa2e52dfe'
export SHA1_KEYCHAIN_NOT_FOUND='f00ecea88ac8e16851779e4230ffd0871c453d40'
export SHA1_COMMON_1='f13bad981225a227a854257d56c0983879ed9733'

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
		echo "unknown operating system."
		exit 1
	;;
esac
export SHA1_BIN
export RANDOM_DEV


typeset -i COUNTER=0
typeset -i RETVAL=0
typeset -i SKIPPED=0
typeset -i START=$SECONDS
sh regress/create_db.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 1
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/ssha.sh; RETVAL=$?;		COUNTER=$(( COUNTER + 1 ))	# 2
	[ $RETVAL -eq 1 ]  &&  exit 1
	[ $RETVAL -eq 2 ]  &&  { SKIPPED=$(( SKIPPED + 1 )); echo skipped; }

sh regress/ykchalresp.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 3
	[ $RETVAL -eq 1 ]  &&  exit 1
	[ $RETVAL -eq 2 ]  &&  { SKIPPED=$(( SKIPPED + 1 )); echo skipped; }

sh regress/cmd_quit.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 4
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_help.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 5
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_version.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 6
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_clear.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 7
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_getnum.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 8
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_list.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 9
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_clist.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 10
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_search.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 11
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_searchre.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 12
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_new.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 13
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_edit.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 14
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_info.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 15
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_swap.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 16
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_insert.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 17
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_export.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 18
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_import.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 19
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_cnew.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 20
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_cedit.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 21
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_cdel.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 22
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_c.sh; RETVAL=$?;		COUNTER=$(( COUNTER + 1 ))	# 23
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_copy.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 24
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_del.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 25
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_write.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 26
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/maxpassword.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 27
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_passwd.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 28
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_status.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 29
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_near.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 30
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_opt_c.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 31
	[ $RETVAL -eq 1 ]  &&  exit 1

sh regress/cmd_clipboard.sh; RETVAL=$?;	COUNTER=$(( COUNTER + 1 ))	# 32
	[ $RETVAL -eq 1 ]  &&  exit 1
	[ $RETVAL -eq 2 ]  &&  { SKIPPED=$(( SKIPPED + 1 )); echo skipped; }

exit 0
