#!/bin/sh -e
set -e


lint -i -hx $(pkg-config --cflags libxml-2.0) $(pkg-config --libs libxml-2.0) \
	$(pkg-config --cflags libpcre) $(pkg-config --libs libpcre) \
	-lssl -lcrypto -ledit -lncursesw \
	-D_HAVE_PCRE $@
