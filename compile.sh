#!/bin/sh -e
set -e


OPTIONS_DEFAULT='HAVE_PCRE=y'

SVN_VERSION=$(svn info |awk '/^Revision:/ {print $2}')
sed -r -i -e "s/dev-SVN_VERSION/dev-${SVN_VERSION}/" common.h

MAKE="make"
case "$(uname -s)" in
	Linux)
		${MAKE} -f GNUMakefile ${OPTIONS_DEFAULT} $@
	;;
	*BSD)
		${MAKE} -f Makefile ${OPTIONS_DEFAULT} $@
	;;
	*)
		echo "unknown system."
	;;
esac

sed -r -i -e "s/dev-${SVN_VERSION}/dev-SVN_VERSION/" common.h
