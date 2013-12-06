#!/bin/sh -e

set -e


echo "test => $0"

if [ -z "${KC_DB}" ]  ||  [ -z ${KC_PASSFILE} ];then
	echo "No database or password file specified!";
	exit 1;
fi

rm -f ${KC_DB} ${KC_PASSFILE}

PASSWORD='qNYHMvXunofKXY7NSBCmDa2T4Av8R2rJWk15ADvca0IHkFqpiqriPnjtQHzWeZBq2MCk3SpoSc6aer5VO33RRC0aM85mxtid50gUUwRT0OhsfKxpTTBdr1hxwQhklpQZj5F28GbDYE5OPWWVKxzmhsbpdt1cfmJw8vG3vn8j3KqcljIn7UdTyJl8yGjtnmWR6wE3G4OW2mEZE8ruX2GnYIHBNwRKc71AakejERXObtdIFQfjxY4V6nyTWPkcPRLT'

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
echo 'FV?Z[,%~d/XDF0eS{w?2NSJ;@KA()}P6ctmK0hGl5qqaCcjKd+5mWIttM66nTl/Bj1XCu/mhqkgcMuWFujYu5pc0X9U3YcNZ
BvuN6sukT523u7OhItYQWBdXucIv0+EMxKqvTMwAN8XyHmIzMo1Xx3RU7pAYOyH0
Dg5FRNpIYPDqJ+jLTkyUF3mws0CFWcKdQl7wGjVqAOIJCruyI84+ar9WYCuyZTAh
AiMCssEl09DG7WqAN7Wkx8dOOczDbZjm8X9XeUMEw6DXugPd8rjVgnLx7L0aK+/T
4SuoGLCJT68+TKc0kYpqaSqJx7NDqeH8ZwVtanetOVMEZZyWd9U3oXrkkIldYIO8
snoMnhZtGOPGOv3NDk1yjEcCmuzJ8X7tBkMF1qdUeR2lod7kLa+/sQt3vC+1iIO2
7LhfTUAT8xrJ5jaO0PlAPTKpzu0/+4KahO41a+Y8Fd40lAhFTSK/bWcjVnbq+YLa
gpYmuECTa42ZCzhz0ZsrSgbDx+PHFpHcideqYdrqdvM=' > ${KC_DB}

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
