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


SHA1=$(printf "info 0\n" |./kc -b -k regress/test -p regress/testpass |grep -E -v -e '^default% >' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'c305cb0a9c3156d287abc4896f9032458c8a1cf1' ];then
	echo "$0 test ok (missing created/modified)!"
else
	echo "$0 test failed (missing created/modified)!"
	exit 1
fi

CREATED=$(printf "info 2\n" |./kc -b -k regress/test -p regress/testpass |grep -E -e '^Created: ' | cut -d' ' -f2-)
MODIFIED=$(printf "info 2\n" |./kc -b -k regress/test -p regress/testpass |grep -E -e '^Modified: ' | cut -d' ' -f2-)
if echo "${CREATED}" |grep -E -q -e '^(Mon|Tue|Wed|Thu|Fri|Sat|Sun) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) [0-9]{2,2} [0-9]{2,2}:[0-9]{2,2}:[0-9]{2,2} [0-9]{4,4}$';then
	echo "$0 test ok (created)!"
else
	echo "$0 test failed (created)!"
	exit 1
fi

if echo "${MODIFIED}" |grep -E -q -e '^(Mon|Tue|Wed|Thu|Fri|Sat|Sun) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) [0-9]{2,2} [0-9]{2,2}:[0-9]{2,2}:[0-9]{2,2} [0-9]{4,4}$';then
	echo "$0 test ok (modified)!"
else
	echo "$0 test failed (modified)!"
	exit 1
fi

exit 0
