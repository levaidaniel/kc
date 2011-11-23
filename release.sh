#!/bin/sh -e
set -e


if [ ! -f common.h ];then
	echo 'wrong pwd?'
	exit 1;
fi

VERSION=$(fgrep VERSION common.h |cut -f3 |cut -d' ' -f2 |tr -d '"')

rm -Rf ../kc-"${VERSION}"
svn export . ../kc-"${VERSION}"
cd ..
tar -zcf kc-"${VERSION}".tgz ./kc-"${VERSION}"
rm -Rf ./kc-"${VERSION}"
