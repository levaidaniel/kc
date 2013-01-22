#!/bin/sh -e

set -e


echo "test => $0"

NAME=$(grep -E -e '^#define[[:space:]]+NAME' common.h |cut -f3 |tr -d '"')
VERSION=$(grep -E -e '^#define[[:space:]]+VERSION' common.h |cut -f3 |tr -d '"')
SVN_VERSION=$(svn info |awk '/^Revision:/ {print $2}')

if printf "version\n" |./kc -b -k regress/test -p regress/testpass |grep -E -e "^${NAME} $(echo ${VERSION} |cut -d- -f1)-$(echo ${VERSION} |cut -d- -f2)-(${SVN_VERSION}|SVN_VERSION)";then
	echo "$0 test ok (version)!"
else
	echo "$0 test failed (version)!"
	exit 1
fi

if printf "version\n" |./kc -b -k regress/test -p regress/testpass |grep -E -e '^Compiled with (Readline|Editline)(, RegExp)* support\.$';then
	echo "$0 test ok (compile options)!"
else
	echo "$0 test failed (compile options)!"
	exit 1
fi

exit 0
