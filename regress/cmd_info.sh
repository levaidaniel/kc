#!/bin/sh -e

set -e


echo "test => $0"


SHA1=$(printf "info\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |sed -e 's/^\(Created: \).*$/\1/' -e 's/^\(Modified: \).*$/\1/' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'bb1acfb7755f2b6c4852268f4f0a6f34ea98c98e' ];then
	echo "$0 test ok (keychain description/created/modified)!"
else
	echo "$0 test failed (keychain description/created/modified)!"
	exit 1
fi

SHA1=$(printf "info 0\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." -e "^Using password file: ${KC_PASSFILE}" |sed -e 's/^\(Created: \).*$/\1/' -e 's/^\(Modified: \).*$/\1/' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'baaa95a6be6964811f8cbc962cb697b3e4dff386' ];then
	echo "$0 test ok (key created/modified)!"
else
	echo "$0 test failed (key created/modified)!"
	exit 1
fi

CREATED=$(printf "info 2\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^Created: ' | cut -d' ' -f2-)
MODIFIED=$(printf "info 2\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^Modified: ' | cut -d' ' -f2-)
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
