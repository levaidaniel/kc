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

SSH_ID='regress/test_id_ssh'
SSH_ID_TYPE='ed25519'
SSH_ID_COMMENT="this is what we're looking for"
rm -f "${SSH_ID}" "${SSH_ID}".pub
${SSH_KEYGEN} -t "${SSH_ID_TYPE}" -C "${SSH_ID_COMMENT}" -N '' -f "${SSH_ID}"
if [ $? -ne 0 ];then
	echo "$0 test failed (generate new ssh key)!"
	exit 1
fi

${SSH_ADD} "${SSH_ID}"
if [ $? -ne 0 ];then
	echo "$0 test failed (add new ssh key to agent)!"
	exit 1
fi

KC_DB='regress/test_ssha.kcd'
rm -f "${KC_DB}"
if echo "write" |${KC_RUN} -A "ssh-${SSH_ID_TYPE},${SSH_ID_COMMENT}" -b -k ${KC_DB};then
	echo "$0 test ok (create new db with agent)!"
else
	echo "$0 test failed (create new db with agent)!"
	exit 1
fi

SHA1=$(echo "status" |${KC_RUN} -A "ssh-${SSH_ID_TYPE},${SSH_ID_COMMENT}" -b -k ${KC_DB} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e '^Database file:' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'c98403e5652a7180fa5594c502649c1c9f7afdd7' ];then
	echo "$0 test ok (status output)!"
else
	echo "$0 test failed (status output)!"
	exit 1
fi

${SSH_ADD} -d ${SSH_ID}
