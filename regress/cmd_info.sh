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


SHA1=$(printf "info\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |sed -e 's/^\(Modified: \).*$/\1/' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'db6f86083e3da30ca234edbe29cfc01f3167488f' ];then
	echo "$0 test ok (keychain description/created/modified)!"
else
	echo "$0 test failed (keychain description/created/modified)!"
	exit 1
fi

SHA1=$(printf "info 0\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = '83b3b5c7809abd12d061eb0c03a39ec135845871' ];then
	echo "$0 test ok (key created/modified)!"
else
	echo "$0 test failed (key created/modified)!"
	exit 1
fi

CREATED=$(printf "info 2\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^Created: ' | cut -d' ' -f2-)
MODIFIED=$(printf "info 2\n" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^Modified: ' | cut -d' ' -f2-)
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
