#!/bin/sh -e

set -e


echo "test => $0"

PASSWORD=$(cat ${KC_PASSFILE})
NEWPASSWORD='123abc321ABC'

if printf "passwd\n${NEWPASSWORD}\n${NEWPASSWORD}\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE};then
	echo "$0 test ok (password file)!"
else
	echo "$0 test failed (password file)!"
	exit 1
fi

echo -n "${NEWPASSWORD}" > ${KC_PASSFILE}

if printf "passwd\n${PASSWORD}\n${PASSWORD}\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE};then
	echo "$0 test ok (new password)!"
else
	echo "$0 test failed (new password)!"
	exit 1
fi

echo -n "${PASSWORD}" > ${KC_PASSFILE}

if printf "passwd -P bcrypt\n${PASSWORD}\n${PASSWORD}\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE};then
	echo "$0 test ok (password file #2)!"
else
	echo "$0 test failed (password file #2)!"
	exit 1
fi


typeset -i SKIP=0
if [ -z "${SSH_AUTH_SOCK}" ];then
	echo "$0 SSH agent test skipped (ssh-agent is not running)!"
	SKIP=1
fi

typeset -i RETVAL=0
which ssh-add  ||  RETVAL=$?
if [ $RETVAL -eq 0 ];then
	SSH_ADD=$(which ssh-add)
else
	echo "$0 SSH agent test skipped (cannot find ssh-add(1))!"
	SKIP=1
fi


if [ $SKIP -eq 0 ];then
	ssh-add regress/test_id_rsa:2048_ssh
	if printf "passwd -A ssh-rsa,PuBkEy_+*()[]{}';.!@#^&=-/|_comment-rsa:2048,password -P bcrypt -e blowfish -m ecb\n${PASSWORD}\n${PASSWORD}\n" |${KC_RUN} -b -k ${KC_DB} -P bcrypt -p ${KC_PASSFILE};then
		echo "$0 test ok (bcrypt KDF)!"
	else
		echo "$0 test failed (bcrypt KDF)!"
		exit 1
	fi

	if printf "${PASSWORD}\n${PASSWORD}\npasswd -A ssh-rsa,PuBkEy_+*()[]{}';.!@#^&=-/|_comment-rsa:2048 -P bcrypt -e blowfish -m ecb\n" |${KC_RUN} -b -k ${KC_DB} -P bcrypt -e blowfish -m ecb -A "ssh-rsa,PuBkEy_+*()[]{}';.!@#^&=-/|_comment-rsa:2048,password";then
		echo "$0 test ok (SSH agent with password, bcrypt KDF, blowfish cipher, ecb cipher mode)!"
	else
		echo "$0 test failed (SSH agent with password, bcrypt KDF, blowfish cipher, ecb cipher mode)!"
		exit 1
	fi

	# Also fake an invalid parameter
	if printf "passwd --help\npasswd -P sha512 -e aes256 -m cbc\n${PASSWORD}\n${PASSWORD}\n" |${KC_RUN} -b -k ${KC_DB} -P bcrypt -e blowfish -m ecb -A "ssh-rsa,PuBkEy_+*()[]{}';.!@#^&=-/|_comment-rsa:2048";then
		echo "$0 test ok (SSH agent, bcrypt KDF, blowfish cipher, ecb cipher mode)!"
	else
		echo "$0 test failed (SSH agent, bcrypt KDF, blowfish cipher, ecb cipher mode)!"
		exit 1
	fi
	ssh-add -d regress/test_id_rsa:2048_ssh

	if echo "" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE};then
		echo "$0 test ok (password, sha512 KDF, aes256 cipher, cbc cipher mode)!"
	else
		echo "$0 test failed (password, sha512 KDF, aes256 cipher, cbc cipher mode)!"
		exit 1
	fi
else
	if printf "passwd -P sha512 -e aes256 -m cbc\n${PASSWORD}\n${PASSWORD}\n" |${KC_RUN} -b -k ${KC_DB} -P bcrypt -p ${KC_PASSFILE};then
		echo "$0 test ok (bcrypt KDF)!"
	else
		echo "$0 test failed (bcrypt KDF)!"
		exit 1
	fi
fi


exit 0
