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

cmd="c newchain\ncedit\n"
if [ ${READLINE} ];then
	cmd=$cmd"renamed_\ndescription\n"
else
	cmd=$cmd"renamed_newchain\ndescription\n"
fi
cmd=$cmd"write\n"

printf "${cmd}" |./kc -b -k regress/test -p regress/testpass

SHA1=$(printf "list\n" |KC_DEBUG=yes ./kc -b -k regress/test -p regress/testpass |grep -E -e '^[[:space:]]*<.*>$' |sed -e 's/ created="[0-9]\{1,\}"//' -e 's/ modified="[0-9]\{1,\}"//' -e 's/ description=".*"//' |$SHA1_BIN |cut -d' ' -f1)
if [ "$SHA1" = 'ffd511e95142efcf312ba543c28209362995755d' ];then
	echo $0 test ok!
else
	echo $0 test failed!
	exit 1
fi

exit 0
