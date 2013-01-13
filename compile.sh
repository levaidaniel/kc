#!/bin/sh -e
set -e


OPTIONS_DEFAULT='HAVE_PCRE=y'

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
