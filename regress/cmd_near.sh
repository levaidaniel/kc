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

printf "new pos3\npos3\nnew pos4\npos4\nnew pos5\npos5\nnew pos6\npos6\nnew pos7\npos7\nwrite\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE}

NEAR0_SHA1=$(printf "near 0\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |$SHA1_BIN |cut -d' ' -f1)
NEAR1_SHA1=$(printf "near 1\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |$SHA1_BIN |cut -d' ' -f1)
NEAR12_SHA1=$(printf "near 1 2\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |$SHA1_BIN |cut -d' ' -f1)
NEAR33_SHA1=$(printf "near 3 3\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |$SHA1_BIN |cut -d' ' -f1)
NEAR73_SHA1=$(printf "near 7 3\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |$SHA1_BIN |cut -d' ' -f1)
NEAR93_SHA1=$(printf "near 9 3\n" |${KC_RUN} -b -k ${KC_DB} -p ${KC_PASSFILE} |grep -E -v -e '^<default% >' -e "^Opening '${KC_DB}'" -e "^Using '${KC_DB}' database." |$SHA1_BIN |cut -d' ' -f1)

if [ "$NEAR0_SHA1" = 'd8ed4d666ce137c19fcc145781978ac84d46221b' ];then
	echo "$0 test ok (near 0)!"
else
	echo "$0 test failed (near 0)!"
	exit 1
fi

if [ "$NEAR1_SHA1" = 'c7b7d0f35a12b8aea88daf3689524a91bc42e402' ];then
	echo "$0 test ok (near 1)!"
else
	echo "$0 test failed (near 1)!"
	exit 1
fi

if [ "$NEAR12_SHA1" = '8a317ddf8e0ed067872f017d96774eff83562218' ];then
	echo "$0 test ok (near 1 2)!"
else
	echo "$0 test failed (near 1 2)!"
	exit 1
fi

if [ "$NEAR33_SHA1" = 'd67d4ace25a94a0dd50a059dc8c4c5e99189c38c' ];then
	echo "$0 test ok (near 3 3)!"
else
	echo "$0 test failed (near 3 3)!"
	exit 1
fi

if [ "$NEAR73_SHA1" = '01583da4c9a6b7ce5f447b7c3f47578e8f24a82b' ];then
	echo "$0 test ok (near 7 3)!"
else
	echo "$0 test failed (near 7 3)!"
	exit 1
fi

if [ "$NEAR93_SHA1" = '812e96292afbdf1b0cebb40a7db6a7ffa2e52dfe' ];then
	echo "$0 test ok (near 9 3)!"
else
	echo "$0 test failed (near 9 3)!"
	exit 1
fi


exit 0
