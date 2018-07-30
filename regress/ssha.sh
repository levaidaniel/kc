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
for SSH_ID_TYPE in ecdsa ed25519 rsa;do
	SSH_ID_FILE="regress/test_id_${SSH_ID_TYPE}_ssh"
	SSH_ID_COMMENT="this is what we're looking for"

	if [ "${SSH_ID_TYPE}" == 'ecdsa' ];then
		KC_ID_TYPE='ecdsa-sha2-nistp256'
	else
		KC_ID_TYPE="ssh-${SSH_ID_TYPE}"
	fi

	rm -f "${SSH_ID_FILE}" "${SSH_ID_FILE}".pub

	${SSH_KEYGEN} -o -t "${SSH_ID_TYPE}" -C "${SSH_ID_COMMENT}" -N '' -f "${SSH_ID_FILE}"
	if [ $? -ne 0 ];then
		echo "$0 test failed (generate new ssh key)!"
		exit 1
	fi

	${SSH_ADD} "${SSH_ID_FILE}"
	if [ $? -ne 0 ];then
		echo "$0 test failed (add new ssh key to agent)!"
		exit 1
	fi

	rm -f "${KC_DB}"
	if echo "write" |${KC_RUN} -A "${KC_ID_TYPE},${SSH_ID_COMMENT}" -b -k ${KC_DB};then
		echo "$0 test ok (create new db with agent, key type ${SSH_ID_TYPE})!"
	else
		echo "$0 test failed (create new db with agent, key type ${SSH_ID_TYPE})!"
		exit 1
	fi

	${SSH_ADD} -d ${SSH_ID_FILE}
done
