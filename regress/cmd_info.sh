#!/bin/sh -e

set -e


echo "test => $0"

case "$(uname -s)" in
	Linux)
		SHA1_BIN=$(which sha1sum)
	;;
	*BSD)
		SHA1_BIN="$(which sha1) -r"
	;;
	*)
		echo "unknown system."
		exit 1
	;;
esac


SHA1=$(printf "info\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |sed -e 's/^\(Modified: \).*$/\1/' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'd559fc302db736029461eccc8a5232bc17678224' ];then
	echo "$0 test ok (keychain description/created/modified)!"
else
	echo "$0 test failed (keychain description/created/modified)!"
	exit 1
fi

SHA1=$(printf "info 0\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^<default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '4a40a905ddda3754fe05ac8e0b98681e0e7f35f6' ];then
	echo "$0 test ok (key created/modified)!"
else
	echo "$0 test failed (key created/modified)!"
	exit 1
fi

CREATED=$(printf "info 2\n" |./kc -b -k regress/test -p regress/testpass |grep -E -e '^Created: ' | cut -d' ' -f2-)
MODIFIED=$(printf "info 2\n" |./kc -b -k regress/test -p regress/testpass |grep -E -e '^Modified: ' | cut -d' ' -f2-)
if echo "${CREATED}" |grep -E -q -e '^(Mon|Tue|Wed|Thu|Fri|Sat|Sun) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) {1,2}[0-9]{1,2} [0-9]{2,2}:[0-9]{2,2}:[0-9]{2,2} [0-9]{4,4}$';then
	echo "$0 test ok (created)!"
else
	echo "$0 test failed (created)!"
	exit 1
fi

if echo "${MODIFIED}" |grep -E -q -e '^(Mon|Tue|Wed|Thu|Fri|Sat|Sun) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) {1,2}[0-9]{1,2} [0-9]{2,2}:[0-9]{2,2}:[0-9]{2,2} [0-9]{4,4}$';then
	echo "$0 test ok (modified)!"
else
	echo "$0 test failed (modified)!"
	exit 1
fi

exit 0
