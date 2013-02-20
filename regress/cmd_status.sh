#!/bin/sh -e

set -e


echo "test => $0"


database_name=$(echo "status" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |grep -E -e '^Database file:' |sed -e 's/ (.*)$//')
cipher_mode=$(echo "status" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |grep -E -e '^Cipher mode:')
keychains=$(echo "status" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |grep -E -e '^Keychains: ')
items_all=$(echo "status" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |grep -E -e '^Items \(all\): ')
read_only=$(echo "status" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |grep -E -e '^Read-only: ')
modified=$(echo "status" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |grep -E -e '^Modified: ')
xml_size=$(echo "status" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |grep -E -e '^XML structure size \(bytes\): ')

if [ "$database_name" = 'Database file: regress/test' ];then
	echo "$0 test ok (database name)!"
else
	echo "$0 test failed (database name)!"
	exit 1
fi

if [ "$cipher_mode" = 'Cipher mode: cbc' ];then
	echo "$0 test ok (cipher mode)!"
else
	echo "$0 test failed (cipher mode)!"
	exit 1
fi

if [ "$keychains" = 'Keychains: 3' ];then
	echo "$0 test ok (keychains)!"
else
	echo "$0 test failed (keychains)!"
	exit 1
fi

if [ "$items_all" = 'Items (all): 5' ];then
	echo "$0 test ok (items all)!"
else
	echo "$0 test failed (items all)!"
	exit 1
fi

if [ "$read_only" = 'Read-only: no' ];then
	echo "$0 test ok (readonly)!"
else
	echo "$0 test failed (readonly)!"
	exit 1
fi

if [ "$modified" = 'Modified: no' ];then
	echo "$0 test ok (modified)!"
else
	echo "$0 test failed (modified)!"
	exit 1
fi

if [ "$xml_size" = 'XML structure size (bytes): 532' ];then
	echo "$0 test ok (xml size)!"
else
	echo "$0 test failed (xml size)!"
	exit 1
fi

exit 0
