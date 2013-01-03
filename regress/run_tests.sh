#!/bin/sh -e

set -e


if [ $(basename $(pwd))  == 'regress' ];then
	echo "please run this script directly from the source directory."
	exit 1
fi

[ "$1" == 'readline' ]  &&  export READLINE=readline

sh regress/create_db.sh

sh regress/cmd_quit.sh
sh regress/cmd_help.sh
sh regress/cmd_version.sh
sh regress/cmd_random.sh
sh regress/cmd_clear.sh

sh regress/cmd_getnum.sh

sh regress/cmd_list.sh
sh regress/cmd_clist.sh

sh regress/cmd_search.sh
sh regress/cmd_searchre.sh

sh regress/cmd_new.sh
sh regress/cmd_edit.sh

sh regress/cmd_export.sh
sh regress/cmd_import.sh

sh regress/cmd_cnew.sh
sh regress/cmd_c.sh
sh regress/cmd_cren.sh
sh regress/cmd_cdel.sh

sh regress/cmd_copy.sh
sh regress/cmd_del.sh

sh regress/cmd_write.sh
sh regress/cmd_passwd.sh

printf "\nAll tests ok! :)\n"

rm -f regress/test*
