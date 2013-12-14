#!/bin/sh -e

set -e


if [ $(basename $(pwd))  = 'regress' ];then
	echo "please run this script directly from the source directory."
	exit 1
fi

if ./kc -v |grep -E -q -e '^Compiled with Readline(, PCRE)* support\.$';then
	export READLINE=readline
fi

export KC_DB='regress/test.kcd'
export KC_PASSFILE='regress/testpass'

COUNTER=0

TESTS=$(ls -1 regress/*.sh |grep -F -v -e"stress_test.sh" -e"run_tests.sh" |wc -l)
trap 'printf "\nTest #$COUNTER (out of $TESTS) failed! :(\n" 1>&2' ERR


sh regress/create_db.sh; COUNTER=$(( COUNTER + 1 ))

sh regress/cmd_quit.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_help.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_version.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_clear.sh; COUNTER=$(( COUNTER + 1 ))

sh regress/cmd_getnum.sh; COUNTER=$(( COUNTER + 1 ))

sh regress/cmd_list.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_clist.sh; COUNTER=$(( COUNTER + 1 ))

sh regress/cmd_search.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_searchre.sh; COUNTER=$(( COUNTER + 1 ))

sh regress/cmd_new.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_edit.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_info.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_swap.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_insert.sh; COUNTER=$(( COUNTER + 1 ))

sh regress/cmd_export.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_import.sh; COUNTER=$(( COUNTER + 1 ))

sh regress/cmd_cnew.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_cedit.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_cdel.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_c.sh; COUNTER=$(( COUNTER + 1 ))

sh regress/cmd_copy.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_del.sh; COUNTER=$(( COUNTER + 1 ))

sh regress/cmd_write.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/maxpassword.sh; COUNTER=$(( COUNTER + 1 ))
sh regress/cmd_passwd.sh; COUNTER=$(( COUNTER + 1 ))

sh regress/cmd_status.sh; COUNTER=$(( COUNTER + 1 ))

printf "\nAll tests were ok! :)\n"
