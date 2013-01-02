#!/bin/sh -e

set -e


echo "test => $0"

NAME=$(grep -E -e '^#define[[:space:]]+NAME' common.h |cut -f3 |tr -d '"')
VERSION=$(grep -E -e '^#define[[:space:]]+VERSION' common.h |cut -f3 |tr -d '"')

if printf "version\n" |./kc -b -k regress/test -p regress/testpass |grep -E -e "^${NAME} ${VERSION}";then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
