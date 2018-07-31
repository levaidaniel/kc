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
${SSH_ADD} -D
for _type in ed25519:256 rsa:1024 rsa:2048 rsa:4096;do
	SSH_ID_BITS="-b ${_type##*:}"
	SSH_ID_TYPE=${_type%%:*}
	KC_ID_TYPE="ssh-${SSH_ID_TYPE}"
	SSH_ID_FILE="regress/test_id_${_type}_ssh"
	SSH_ID_COMMENT="PuBkEy_+*()[]{}';,.!@#^&=-/|_comment-${_type}"

	rm -f "${SSH_ID_FILE}" "${SSH_ID_FILE}".pub

	${SSH_KEYGEN} -o -t "${SSH_ID_TYPE}" ${SSH_ID_BITS} -C "${SSH_ID_COMMENT}" -N '' -f "${SSH_ID_FILE}"
	if [ $? -ne 0 ];then
		echo "$0 test failed (generate new ssh key, key type ${_type})!"
		exit 1
	fi

	${SSH_ADD} "${SSH_ID_FILE}"
	if [ $? -ne 0 ];then
		echo "$0 test failed (add new ssh key to agent, key type ${_type})!"
		exit 1
	fi

	rm -f "${KC_DB}"
	if echo "write" |${KC_RUN} -A "${KC_ID_TYPE},${SSH_ID_COMMENT}" -b -k ${KC_DB};then
		echo "$0 test ok (create new db with agent, key type ${_type})!"
	else
		echo "$0 test failed (create new db with agent, key type ${_type})!"
		exit 1
	fi

	rm -f regress/test_export.kcd
	if echo "export -A ${KC_ID_TYPE},${SSH_ID_COMMENT} -k regress/test_export" |${KC_RUN} -A "${KC_ID_TYPE},${SSH_ID_COMMENT}" -b -k ${KC_DB};then
		echo "$0 test ok (export#1, key type ${_type})!"
	else
		echo "$0 test failed (export#1, key type ${_type})!"
		exit 1
	fi

	if [ ! -r "regress/test_export.kcd" ];then
		echo "$0 test failed (unreadable export file, key type ${_type})!"
		exit 1
	fi

	if echo exit |${KC_RUN} -A "${KC_ID_TYPE},${SSH_ID_COMMENT}" -b -k regress/test_export.kcd;then
		echo "$0 test ok (export#2, key type ${_type})!"
	else
		echo "$0 test failed (export#2, key type ${_type})!"
		exit 1
	fi

	if printf "import -A ${KC_ID_TYPE},${SSH_ID_COMMENT} -k regress/test_export.kcd\nwrite\n" |${KC_RUN} -A "${KC_ID_TYPE},${SSH_ID_COMMENT}" -b -k regress/test_export.kcd;then
		echo "$0 test ok (export, key type ${_type})!"
	else
		echo "$0 test failed (export, key type ${_type})!"
		exit 1
	fi
done
${SSH_ADD} -D
