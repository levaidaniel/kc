#!/bin/sh -e
set -e


if [ ! -f common.h ];then
	echo 'wrong pwd?'
	exit 1;
fi

NAME=kc
VERSION=$(fgrep VERSION common.h |cut -f3 |tr -d '"')

rm -Rf ../"${NAME}"-"${VERSION}"
svn export . ../"${NAME}"-"${VERSION}"
cd ..
tar -vzcf "${NAME}"-"${VERSION}".tar.gz ./"${NAME}"-"${VERSION}"
rm -Rf ./"${NAME}"-"${VERSION}"
