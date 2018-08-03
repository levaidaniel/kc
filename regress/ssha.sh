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

PASSWORD='asdbqwdoijqw2189'
KC_DB_SSHA='regress/test_ssha.kcd'
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

	rm -f "${KC_DB_SSHA}"
	if echo "write" |${KC_RUN} -A "${KC_ID_TYPE},${SSH_ID_COMMENT}" -b -k ${KC_DB_SSHA};then
		echo "$0 test ok (create new db with agent, key type ${_type})!"
	else
		echo "$0 test failed (create new db with agent, key type ${_type})!"
		exit 1
	fi

	rm -f regress/test_export.kcd
	if printf "export -k regress/test_export\n${PASSWORD}\n${PASSWORD}\n" |${KC_RUN} -A "${KC_ID_TYPE},${SSH_ID_COMMENT}" -b -k ${KC_DB_SSHA};then
		echo "$0 test ok (export#1 from agent to non-agent, key type ${_type})!"

		if printf "${PASSWORD}\nlist\nexit\n" |${KC_RUN} -b -k regress/test_export.kcd;then
			echo "$0 test ok (opening after export#1 from agent to non-agent, key type ${_type})!"
		else
			echo "$0 test failed (opening after export#1 from agent to non-agent, key type ${_type})!"
			exit 1
		fi
	else
		echo "$0 test failed (export#1 from agent to non-agent, key type ${_type})!"
		exit 1
	fi

	if [ ! -r "regress/test_export.kcd" ];then
		echo "$0 test failed (export#1 from agent to non-agent, unreadable export file, key type ${_type})!"
		exit 1
	fi

	rm -f regress/test_export.kcd
	if echo "export -A ${KC_ID_TYPE},${SSH_ID_COMMENT} -k regress/test_export" |${KC_RUN} -A "${KC_ID_TYPE},${SSH_ID_COMMENT}" -b -k ${KC_DB_SSHA};then
		echo "$0 test ok (export#1 from agent to agent, key type ${_type})!"

		if printf "list\nexit\n" |${KC_RUN} -A ${KC_ID_TYPE},${SSH_ID_COMMENT} -b -k regress/test_export.kcd;then
			echo "$0 test ok (opening after export#1 from agent to agent, key type ${_type})!"
		else
			echo "$0 test failed (opening after export#1 from agent to agent, key type ${_type})!"
			exit 1
		fi
	else
		echo "$0 test failed (export#1 from agent to agent, key type ${_type})!"
		exit 1
	fi

	if [ ! -r "regress/test_export.kcd" ];then
		echo "$0 test failed (export#1 from agent to agent, unreadable export file, key type ${_type})!"
		exit 1
	fi

	rm -f regress/test_export.kcd
	if echo "export -A ${KC_ID_TYPE},${SSH_ID_COMMENT} -k regress/test_export" |${KC_RUN} -p ${KC_PASSFILE} -b -k ${KC_DB};then
		echo "$0 test ok (export#1 from non-agent to agent, key type ${_type})!"

		if printf "list\nexit\n" |${KC_RUN} -A ${KC_ID_TYPE},${SSH_ID_COMMENT} -b -k regress/test_export.kcd;then
			echo "$0 test ok (opening after export#1 from non-agent to agent, key type ${_type})!"
		else
			echo "$0 test failed (opening after export#1 from non-agent to agent, key type ${_type})!"
			exit 1
		fi
	else
		echo "$0 test failed (export#1 from non-agent to agent, key type ${_type})!"
		exit 1
	fi

	if [ ! -r "regress/test_export.kcd" ];then
		echo "$0 test failed (export#1 from non-agent to agent, unreadable export file, key type ${_type})!"
		exit 1
	fi

	if printf "import -A ${KC_ID_TYPE},${SSH_ID_COMMENT} -k regress/test_export.kcd\nwrite\n" |${KC_RUN} -A "${KC_ID_TYPE},${SSH_ID_COMMENT}" -b -k regress/test_export.kcd;then
		echo "$0 test ok (importing export, key type ${_type})!"
	else
		echo "$0 test failed (importing export, key type ${_type})!"
		exit 1
	fi
done
${SSH_ADD} -D
