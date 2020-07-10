#!/bin/sh -e

set -e


echo "test => $0"

if [ -z "${TMUX}" ];then
	echo "$0 tmux(1) doesn't seem to be running, skipping"
	exit 2
fi

TMUX_CMD=$(which tmux)

echo "tmux 0" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}
TMUX_BUF="$(${TMUX_CMD} show-buffer)"
if [ "${TMUX_BUF}" == 'defaultvalue0' ];then
	echo "$0 test ok (tmux)!"
else
	echo "$0 test failed (tmux)!"
	exit 1
fi
${TMUX_CMD} delete-buffer

exit 0
