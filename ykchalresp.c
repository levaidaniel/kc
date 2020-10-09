/*
 * Copyright (c) 2020 LEVAI Daniel
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *	* Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 *	* Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Bits and pieces -- heck, whole functions taken from the
 * yubikey-personalization repository, namely ykchalresp.c */
/*
 * Copyright (c) 2011-2013 Yubico AB.
 * All rights reserved.
 *
 * Author : Fredrik Thulin <fredrik@yubico.com>
 *
 * Some basic code copied from ykpersonalize.c.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "ykchalresp.h"


#define	SHA1_MAX_BLOCK_SIZE	64	/* Max size of input SHA1 block */


static int yk_check_firmware(YK_KEY *);
static void yk_report_error(yk_array *);


int
kc_ykchalresp(struct db_parameters *db_params)
{
	YK_KEY		*yk_key = 0;
	int		yk_cmd = 0;
	yk_array	*yk = NULL;

	bool		may_block = true;

	unsigned char	response[SHA1_MAX_BLOCK_SIZE];

	unsigned char	*challenge = NULL, *passtmp = NULL;
	size_t		challenge_len = 0, passtmp_len = 0;


	yk_errno = 0;

	if (!yk_init()) {
		goto err;
	}

	yk = db_params->yk;
	while (yk) {
		printf("Using YubiKey slot #%d on device #%d%s\n", yk->yk_slot, yk->yk_dev, (yk->yk_password ? " and a password" : ""));

		if (getenv("KC_DEBUG"))
			printf("%s() yk array:\nyk->yk_slot = %d\nyk->yk_dev = %d\nyk->yk_password = %d\n", __func__, yk->yk_slot, yk->yk_dev, yk->yk_password);

		if (!(yk_key = yk_open_key((int)yk->yk_dev))) {
			goto err;
		}

		if (!yk_check_firmware(yk_key)) {
			goto err;
		}

		if (yk->yk_password  &&  db_params->pass_len > YUBIKEY_CHALLENGE_MAXLEN) {
			dprintf(STDERR_FILENO, "ERROR: Password cannot be longer than %d bytes when using YubiKey challenge-response!\n", YUBIKEY_CHALLENGE_MAXLEN);
			yk_errno = YK_EWRONGSIZ;
			goto err;
		}

		switch(yk->yk_slot) {
			case 1:
				yk_cmd = SLOT_CHAL_HMAC1;
				break;
			case 2:
				yk_cmd = SLOT_CHAL_HMAC2;
				break;
			default:
				goto err;
		}


		/* set up challenge */
		if (yk->yk_password) {
			if (getenv("KC_DEBUG"))
				printf("%s(): using password with yubikey\n", __func__);

			/* Here we not only copy the user-supplied password to the
			 * challenge, but also append data from the salt -- as much
			 * data as there is space left from YUBIKEY_CHALLENGE_MAXLEN
			 * after appending the user-supplied password.
			 * Or if for some reason we could copy less from the salt, then
			 * prioritize the SALT_LEN and only copy as much as we can from
			 * the salt.
			 */
			challenge_len = db_params->pass_len + SALT_LEN > YUBIKEY_CHALLENGE_MAXLEN ? YUBIKEY_CHALLENGE_MAXLEN : db_params->pass_len + SALT_LEN;
			challenge = malloc(challenge_len); malloc_check(challenge);
			memcpy(challenge, db_params->pass, db_params->pass_len);
			memcpy(challenge + db_params->pass_len, db_params->salt, db_params->pass_len + SALT_LEN > challenge_len ? challenge_len - db_params->pass_len : SALT_LEN);
			if (getenv("KC_DEBUG"))
				printf("%s(): the challenge is filled with %zu bytes from salt after %zu bytes of password\n", __func__, challenge_len - db_params->pass_len, db_params->pass_len);

			/* save the password temporarily, so that we can append it
			 * later after the response in the new constructed password */
			passtmp = malloc(db_params->pass_len); malloc_check(passtmp);
			passtmp_len = db_params->pass_len;
			memcpy(passtmp, db_params->pass, db_params->pass_len);
		} else {
			if (getenv("KC_DEBUG"))
				printf("%s(): automatic yubikey, without password\n", __func__);

			/* this won't be the digest of the salt, but the actual decoded
			 * salt */
			challenge_len = SALT_LEN;
			challenge = malloc(challenge_len); malloc_check(challenge);
			yubikey_hex_decode((char *)challenge, (char *)db_params->salt, challenge_len);
			if (getenv("KC_DEBUG"))
				printf("%s(): the challenge is filled with %zu bytes from salt\n", __func__, challenge_len);
		}


		printf("Remember to touch your YubiKey if necessary\n");
		memset(response, 0, sizeof(response));
		if (!yk_challenge_response(yk_key, yk_cmd, may_block,
			challenge_len, challenge,
			sizeof(response), response))
		{
			goto err;
		}

		/* realloc ..->pass */
		memset(db_params->pass, '\0', db_params->pass_len);
		db_params->pass_len = sizeof(response) + passtmp_len > PASSWORD_MAXLEN ? PASSWORD_MAXLEN : sizeof(response) + passtmp_len;
		db_params->pass = realloc(db_params->pass, db_params->pass_len); malloc_check(db_params->pass);

		/* copy the response as the constructed password */
		memcpy(db_params->pass, response, sizeof(response) > db_params->pass_len ? db_params->pass_len : sizeof(response));
		memset(response, 0, sizeof(response));

		/* if there's a password, then append it to the response */
		if (yk->yk_password  &&  passtmp) {
			if (getenv("KC_DEBUG"))
				printf("%s(): constructing new password by appending password to yubikey response\n", __func__);

			/* append the actual user password as well to the end of the constructed password */
			memcpy(db_params->pass + sizeof(response), passtmp, sizeof(response) + passtmp_len > db_params->pass_len ? db_params->pass_len - sizeof(response) : passtmp_len);
		}

		yk = yk->next;
	}

err:
	if (passtmp)
		memset(passtmp, '\0', passtmp_len);
	free(passtmp); passtmp = NULL;
	passtmp_len = 0;

	yk_report_error(yk);
	if (yk_key && !yk_close_key(yk_key)) {
		yk_report_error(yk);
	}

	if (yk_errno) {
		return(0);
	} else {
		return(1);
	}
} /* kc_ykchalresp() */


static int
yk_check_firmware(YK_KEY *yk_key)
{
	YK_STATUS *st = ykds_alloc();

	if (!yk_get_status(yk_key, st)) {
		ykds_free(st);
		return 0;
	}

	if (ykds_version_major(st) < 2 ||
	    (ykds_version_major(st) == 2
	     && ykds_version_minor(st) < 2)) {
		fprintf(stderr, "Challenge-response not supported before YubiKey 2.2.\n");
		ykds_free(st);
		return 0;
	}

	ykds_free(st);
	return 1;
}/* yk_check_firmware() */


static void
yk_report_error(yk_array *yk)
{
	if (yk_errno) {
		if (yk_errno == YK_EUSBERR) {
			fprintf(stderr, "USB error: %s\n",
				yk_usb_strerror());
		} else {
			fprintf(stderr, "YubiKey core error (Device#%d, Slot#%d): %s\n",
				yk->yk_dev, yk->yk_slot, yk_strerror(yk_errno));
		}
	}
}/* yk_report_error */
