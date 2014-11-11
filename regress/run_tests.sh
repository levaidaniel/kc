#!/bin/sh -e

set -e


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

COUNTER=1

TESTS=$(ls -1 regress/*.sh |grep -F -v -e"stress_test.sh" -e"run_tests.sh" |wc -l |tr -d ' ')
trap '
if [ $? -eq 0 ];then
	printf "\nAll tests were ok! :)\n"
else
	printf "\nTest #$COUNTER (out of $TESTS) failed! :(\n" 1>&2
fi
' EXIT

export SHA1_INVALID_INDEX='812e96292afbdf1b0cebb40a7db6a7ffa2e52dfe'
export SHA1_KEYCHAIN_NOT_FOUND='f00ecea88ac8e16851779e4230ffd0871c453d40'
export SHA1_COMMON_1='a15dd0ffd1aef152f7d10654361c01ea1c04dfe5'

sh regress/create_db.sh; COUNTER=$(( COUNTER + 1 ))	# 1
sh regress/cmd_quit.sh; COUNTER=$(( COUNTER + 1 ))	# 2
sh regress/cmd_help.sh; COUNTER=$(( COUNTER + 1 ))	# 3
sh regress/cmd_version.sh; COUNTER=$(( COUNTER + 1 ))	# 4
sh regress/cmd_clear.sh; COUNTER=$(( COUNTER + 1 ))	# 5
sh regress/cmd_getnum.sh; COUNTER=$(( COUNTER + 1 ))	# 6
sh regress/cmd_list.sh; COUNTER=$(( COUNTER + 1 ))	# 7
sh regress/cmd_clist.sh; COUNTER=$(( COUNTER + 1 ))	# 8
sh regress/cmd_search.sh; COUNTER=$(( COUNTER + 1 ))	# 9
sh regress/cmd_searchre.sh; COUNTER=$(( COUNTER + 1 ))	# 10
sh regress/cmd_new.sh; COUNTER=$(( COUNTER + 1 ))	# 11
sh regress/cmd_edit.sh; COUNTER=$(( COUNTER + 1 ))	# 12
sh regress/cmd_info.sh; COUNTER=$(( COUNTER + 1 ))	# 13
sh regress/cmd_swap.sh; COUNTER=$(( COUNTER + 1 ))	# 14
sh regress/cmd_insert.sh; COUNTER=$(( COUNTER + 1 ))	# 15
sh regress/cmd_export.sh; COUNTER=$(( COUNTER + 1 ))	# 16
sh regress/cmd_import.sh; COUNTER=$(( COUNTER + 1 ))	# 17
sh regress/cmd_cnew.sh; COUNTER=$(( COUNTER + 1 ))	# 18
sh regress/cmd_cedit.sh; COUNTER=$(( COUNTER + 1 ))	# 19
sh regress/cmd_cdel.sh; COUNTER=$(( COUNTER + 1 ))	# 20
sh regress/cmd_c.sh; COUNTER=$(( COUNTER + 1 ))		# 21
sh regress/cmd_copy.sh; COUNTER=$(( COUNTER + 1 ))	# 22
sh regress/cmd_del.sh; COUNTER=$(( COUNTER + 1 ))	# 23
sh regress/cmd_write.sh; COUNTER=$(( COUNTER + 1 ))	# 24
sh regress/maxpassword.sh; COUNTER=$(( COUNTER + 1 ))	# 25
sh regress/cmd_passwd.sh; COUNTER=$(( COUNTER + 1 ))	# 26
sh regress/cmd_status.sh; COUNTER=$(( COUNTER + 1 ))	# 27
sh regress/cmd_near.sh; COUNTER=$(( COUNTER + 1 ))	# 28
sh regress/cmd_opt_c.sh; COUNTER=$(( COUNTER + 1 ))	# 29
