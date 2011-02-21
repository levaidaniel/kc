#!/bin/sh -e
set -e


case "$(uname -s)" in
	Linux)
		make -f GNUMakefile install
	;;
	*BSD)
		make -f Makefile install
	;;
	*)
		echo "unknown system."
	;;
esac
type kc
