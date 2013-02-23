#!/bin/sh -e

set -e


echo "test => $0"

NAME=$(grep -E -e '^#define[[:space:]]+NAME' common.h |cut -f3 |tr -d '"')

if printf "version\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e "^${NAME} [[:alnum:]\.\-]+";then
	echo "$0 test ok (version)!"
else
	echo "$0 test failed (version)!"
	exit 1
fi

if printf "version\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^Compiled with (Readline|Editline)(, PCRE)* support\.$';then
	echo "$0 test ok (compile options)!"
else
	echo "$0 test failed (compile options)!"
	exit 1
fi

exit 0
