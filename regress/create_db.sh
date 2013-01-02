#!/bin/sh -e

set -e


echo "test => $0"

rm -f regress/test regress/testpass

PASSWORD='abc123ABC321'

# create a random database
if printf "${PASSWORD}\n${PASSWORD}\n" |./kc -b -k regress/test;then
	echo "$0 test ok (create)!"
else
	echo "$0 test failed (create)!"
	exit 1
fi

echo ${PASSWORD} > regress/testpass
if echo "" |./kc -b -k regress/test -p regress/testpass;then
	echo "$0 test ok (reopen)!"
else
	echo "$0 test failed (reopen)!"
	exit 1
fi

# create our sample database
echo '*jlPP?4?G:_*ug!_obw1}5!VOt$8V(+su2RVDnH49giCwhkwi0ZY0W7GpGZwB6fX6fONYPCImpC7BteTGy1wI2CxMXUsDH25
XQa4ywI0HG8cIBx230sh+i9tzBYdk+wBLn+dgqwzK7rVCy4vzWS659o78IBGxaxg
kfi1MCBzuaQalSG78rAOSVKDrkRIXEhYu5LIezWXsDQ=' > regress/test

if printf "${PASSWORD}" |./kc -b -k regress/test;then
	echo "$0 test ok (reopen)!"
else
	echo "$0 test failed (reopen)!"
	exit 1
fi

exit 0
