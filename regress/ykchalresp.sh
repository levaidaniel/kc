#!/bin/sh -e

set -e


echo "test => $0"

typeset -i SKIP=0
typeset -i YKVER_MAJ=0
typeset -i YKVER_MIN=0
typeset -i RETVAL=0

if ! printf "version\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^Compiled with (Readline|Editline)(, PCRE)*(, SCRYPT)*(, YUBIKEY)+ support\.$';then
	SKIP=1
	echo "$0 test skipped (kc was not compiled with Yubikey support)!"
fi

which ykinfo  ||  RETVAL=$?
if [ $RETVAL -eq 0 ];then
	YKINFO=$(which ykinfo)
	YKVER=$($YKINFO -v |cut -d' ' -f2)
	YKVER_MAJ=$(echo "${YKVER}" |cut -d. -f1)
	YKVER_MIN=$(echo "${YKVER}" |cut -d. -f2)
else
	SKIP=1
	echo "$0 test skipped (ykinfo not found)!"
fi

if [ ${YKVER_MAJ} -lt 2  -a  ${YKVER_MAJ} -lt 2 ];then
	SKIP=1
	echo "$0 test skipped (${YKVER_MAJ}.${YKVER_MIN} < 2.2)!"
fi


if [ $SKIP -ne 0 ];then
	exit 2
fi


PASSWORD='asdbqwdoijqw2189'
KC_DB_YK='regress/test_yk.kcd'
rm -f "${KC_DB_YK}"

printf "${PASSWORD}\n${PASSWORD}\nwrite\n" |${KC_RUN} -y 2,password -b -k ${KC_DB_YK}
if [ $? -eq 0 ];then
	echo "$0 test ok (create new db, with password)!"
else
	echo "$0 test failed (create new db, with password)!"
	exit 1
fi

if echo "${PASSWORD}" |${KC_RUN} -y 2,password -b -k ${KC_DB_YK};then
	echo "$0 test ok (open db, with password)!"
else
	echo "$0 test failed (open db, with password)!"
	exit 1
fi

if printf "${PASSWORD}\npasswd -y 2\n" |${KC_RUN} -y 2,password -b -k ${KC_DB_YK};then
	echo "$0 test ok (passwd change, without password)!"
else
	echo "$0 test failed (passwd change, without password)!"
	exit 1
fi

if echo "write" |${KC_RUN} -y 2 -b -k ${KC_DB_YK};then
	echo "$0 test ok (open db, without password)!"
else
	echo "$0 test failed (open db, without password)!"
	exit 1
fi


exit 0
