#!/bin/sh -e

set -e


echo "test => $0"

case "$(uname -s)" in
	Linux|CYGWIN*)
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

cmd="c newchain\ncedit\n"
if [ ${READLINE} ];then
	cmd=$cmd"renamed_\nrenamed_\n"
else
	cmd=$cmd"renamed_newchain\nrenamed_description\n"
fi
cmd=$cmd"write\n"

printf "${cmd}" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

SHA1=$(printf "list\n" |KC_DEBUG=yes ${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'c2e5f570bb6bd57f86c02e1420e9de82ab712ae9' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
