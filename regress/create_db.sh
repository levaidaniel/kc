#!/bin/sh -e

set -e


echo "test => $0"

if [ -z "${KC_DB}" ]  ||  [ -z ${KC_PASSFILE} ];then
	echo "No database or password file specified!";
	exit 1;
fi

rm -f ${KC_DB} ${KC_PASSFILE}

PASSWORD='abc123ABC321'

# create a random database
if printf "${PASSWORD}\n${PASSWORD}\n" |./kc -b -k ${KC_DB};then
	echo "$0 test ok (create random db)!"
else
	echo "$0 test failed (create random db)!"
	exit 1
fi

echo ${PASSWORD} > ${KC_PASSFILE}
if echo "" |./kc -b -k ${KC_DB} -p ${KC_PASSFILE};then
	echo "$0 test ok (open random db)!"
else
	echo "$0 test failed (open random db)!"
	exit 1
fi

rm -f ${KC_DB}


# create our sample database
echo '*]duY3#42y/qA8}%~e5T3,~[+sj+i@hSoTvN+zSFDyrnv4qupDxDaq13YJ0NQmn4xQfPkOD7TE1Ow4T9iAvu+niN+yiYsdwJ
oVJQPieg3J5TzwuWo4LAF+9ynn7DGll8+YqUCpVUqv0GwSJrPBU70CvPeWZnxCzr
I5pynbW1Hy/YW+R3uLhTYpbXB5JC2GDsWB4PZKDOm5ekKCLnb8vD4A62r1H/hNS2
e0N6m8dUXtgbJ6EFZ2g6txHEX3YXi+WsNJGCaBW36t1KMJ+O85GatfqD7FpjOLr2
Sbvp5n2E1O4lSkopNDXuUdgoB9Xp3E5VSP4bjLZVMXEwli0wyl/8Z8tawTwvUJ2W
w7bXPUQNgXFH23KK3E9NWxRIY1+sfIR54Ew2539GTsPsAfGQEXVURUHQES/kOpa1
oD+C3KU3iDu1z+SUWyIRgZJl2Zd3WJ2b6ZV0Nktqtu3ziYjZsRyC6Z3yqqMRWueZ
hkhlsJi3BL9UENDOSyur96goOto14xlb+isUihSwY7k=' > ${KC_DB}

if [ ! -r "${KC_DB}" ];then
	echo "$0 test failed (read sample db)!"
	exit 1
fi

if printf "${PASSWORD}" |./kc -b -k ${KC_DB};then
	echo "$0 test ok (create and open sample db)!"
else
	echo "$0 test failed (create and open sample db)!"
	exit 1
fi

exit 0
