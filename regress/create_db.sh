#!/bin/sh -e

set -e


echo "test => $0"

rm -f regress/test regress/testpass

PASSWORD='abc123ABC321'

# create a random database
if printf "${PASSWORD}\n${PASSWORD}\n" |./kc -b -k regress/test;then
	echo "$0 test ok (create random db)!"
else
	echo "$0 test failed (create random db)!"
	exit 1
fi

echo ${PASSWORD} > regress/testpass
if echo "" |./kc -b -k regress/test -p regress/testpass;then
	echo "$0 test ok (open random db)!"
else
	echo "$0 test failed (open random db)!"
	exit 1
fi

rm -f regress/test


# create our sample database
echo '<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE kc SYSTEM "kc.dtd">
<kc>
	<keychain name="default">
		<key name="defaultkey0" value="defaultvalue0"/>
		<key name="defaultkey1" value="defaultvalue1"/>
	</keychain>
	<keychain name="testchain">
		<key name="keytest0" value="valuetest0"/>
		<key name="keytest1" value="valuetest1"/>
	</keychain>
	<keychain name="emptychain">
	</keychain>
</kc>' > regress/test_db.xml

if [ ! -r "regress/test_db.xml" ];then
	echo "$0 test failed (read sample db xml)!"
	exit 1
fi

echo 'w{CQ5/H^DgPv[$[qHc`*oc*SZH*|G`*.nw33FG5BaciY6pc4aIabq7ORQW6kyZH8VNIEog/32cwnCDsQ0iy+4RU+Z2Y+W9Wx
Zkfqnpdr1boE5lEW869+AmKMxL9OhKcagB6NNiM1SxtSNf7mMtnOI8YYIh/15tfT
7V8SIQYFJmeI8tIpm8NvBsP8ELUXSnlEZVqbvXgC9SRYP6iwUUq8g/N6MMzG+EoS
Y0W/LGOtEzhPBXAwcVp2+akFZ9DR34F2PM/IuauvdW+UZl1I2ObWNPFzde62nMLU
kloxN6u+ReX+Hi37LsWQuL4yzYkl+pN6DH4wh+vamRc6wYxDImVMi5b9K90/VyDB
KrbeoLrGsNAFJ1paFuXinmwn2hVS11+5ayEfadHgpsi0cm4Ze9xoroM0eX+Vj7LT
FIOgRFau1xvBM2kauEalqRwYKTjOmTdv41iEKK+FaDhhYl2OId5vbHJSp52OF51s
iRaLY/GO3OsRjdwAOVC8byYKJMWmeAtqoEbDVYfSANuSmDp85XJSwrQ0HvA+pX9o
O8zhUz1Cy+avitWpRn5RsQ==' > regress/test

if [ ! -r "regress/test" ];then
	echo "$0 test failed (read sample db)!"
	exit 1
fi

if printf "${PASSWORD}" |./kc -b -k regress/test;then
	echo "$0 test ok (create and open sample db)!"
else
	echo "$0 test failed (create and open sample db)!"
	exit 1
fi

exit 0
