#!/bin/sh -e
set -e

VERSION=$(fgrep VERSION common.h |cut -f3 |cut -d' ' -f2 |tr -d '"')

rm -Rf ../kc-"${VERSION}"
svn export . ../kc-"${VERSION}"
cd ..
tar -zcf kc-"${VERSION}".tgz ./kc-"${VERSION}"
rm -Rf ./kc-"${VERSION}"
