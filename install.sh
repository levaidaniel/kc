#!/bin/sh -e
set -e


OPTIONS='install'

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
type kc
