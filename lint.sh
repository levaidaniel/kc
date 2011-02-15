#!/bin/sh -e
set -e


lint -i `pkg-config --cflags libxml-2.0` `pkg-config --cflags libpcre` -hx *.c
