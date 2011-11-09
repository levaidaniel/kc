#!/bin/sh -e
set -e


OPTIONS='HAVE_PCRE=y'

MAKE="make $@"
case "$(uname -s)" in
	Linux)
		${MAKE} -f GNUMakefile ${OPTIONS}
	;;
	*BSD)
		${MAKE} -f Makefile ${OPTIONS}
	;;
	*)
		echo "unknown system."
	;;
esac
