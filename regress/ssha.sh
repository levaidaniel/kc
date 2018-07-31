#!/bin/sh -e

set -e


echo "test => $0"

if [ -z "${SSH_AUTH_SOCK}" ];then
	echo "$0 test failed (ssh-agent is not running)!"
	exit 1
fi

# generate a new private key
SSH_KEYGEN=$(which ssh-keygen)
if [ $? -ne 0 ];then
	echo "$0 test failed (cannot find ssh-keygen(1))!"
	exit 1
fi
SSH_ADD=$(which ssh-add)
if [ $? -ne 0 ];then
	echo "$0 test failed (cannot find ssh-add(1))!"
	exit 1
fi

KC_DB='regress/test_ssha.kcd'
# when this is failing, it's a good idea to: ${SSH_ADD} -D
for SSH_ID_TYPE in ed25519:256 rsa:1024 rsa:2048 rsa:4096;do
	SSH_ID_BITS="-b ${SSH_ID_TYPE##*:}"
	SSH_ID_TYPE=${SSH_ID_TYPE%%:*}
	KC_ID_TYPE="ssh-${SSH_ID_TYPE}"
	SSH_ID_FILE="regress/test_id_${SSH_ID_TYPE}_ssh"
	SSH_ID_COMMENT="this_is_what_we're_looking_for"

	rm -f "${SSH_ID_FILE}" "${SSH_ID_FILE}".pub

	${SSH_KEYGEN} -o -t "${SSH_ID_TYPE}" ${SSH_ID_BITS} -C "${SSH_ID_COMMENT}" -N '' -f "${SSH_ID_FILE}"
	if [ $? -ne 0 ];then
		echo "$0 test failed (generate new ssh key, key type ${SSH_ID_TYPE})!"
		exit 1
	fi

	${SSH_ADD} "${SSH_ID_FILE}"
	if [ $? -ne 0 ];then
		echo "$0 test failed (add new ssh key to agent, key type ${SSH_ID_TYPE})!"
		exit 1
	fi

	rm -f "${KC_DB}"
	if echo "write" |${KC_RUN} -A "${KC_ID_TYPE},${SSH_ID_COMMENT}" -b -k ${KC_DB};then
		echo "$0 test ok (create new db with agent, key type ${SSH_ID_TYPE})!"
	else
		echo "$0 test failed (create new db with agent, key type ${SSH_ID_TYPE})!"
		exit 1
	fi

	rm -f regress/test_export.kcd
	if echo "export -A ${KC_ID_TYPE},${SSH_ID_COMMENT} -k regress/test_export" |${KC_RUN} -A "${KC_ID_TYPE},${SSH_ID_COMMENT}" -b -k ${KC_DB};then
		echo "$0 test ok (export#1, key type ${SSH_ID_TYPE})!"
	else
		echo "$0 test failed (export#1, key type ${SSH_ID_TYPE})!"
		exit 1
	fi

	if [ ! -r "regress/test_export.kcd" ];then
		echo "$0 test failed (unreadable export file, key type ${SSH_ID_TYPE})!"
		exit 1
	fi

	if echo exit |${KC_RUN} -A "${KC_ID_TYPE},${SSH_ID_COMMENT}" -b -k regress/test_export.kcd;then
		echo "$0 test ok (export#2, key type ${SSH_ID_TYPE})!"
	else
		echo "$0 test failed (export#2, key type ${SSH_ID_TYPE})!"
		exit 1
	fi

	if printf "import -A ${KC_ID_TYPE},${SSH_ID_COMMENT} -k regress/test_export.kcd\nwrite\n" |${KC_RUN} -A "${KC_ID_TYPE},${SSH_ID_COMMENT}" -b -k regress/test_export.kcd;then
		echo "$0 test ok (export, key type ${SSH_ID_TYPE})!"
	else
		echo "$0 test failed (export, key type ${SSH_ID_TYPE})!"
		exit 1
	fi

	${SSH_ADD} -d ${SSH_ID_FILE}
done
