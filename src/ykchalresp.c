/*
 * Copyright (c) 2020-2024 LEVAI Daniel
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
	YK_KEY		*yk_key = NULL;
	int		yk_cmd = 0;
	yk_array	*yk = db_params->yk;
	unsigned int	yk_serial = 0;

	bool		may_block = true;

	char		*userpass = NULL;
	unsigned char	response[SHA1_MAX_BLOCK_SIZE];

	unsigned char	*challenge = NULL;
	size_t		challenge_len = 0, copied = 0, userpass_len = 0, yk_counter = 0;


	yk_errno = 0;

	if (!yk_init()) {
		goto err;
	}

	/* If we have a user-password, then save it for later, because this is
	 * only available before we do any magic below, because we actually
	 * overwrite this with the YubiKey response.
	 */
	if (db_params->yk_password) {
		userpass = malloc(db_params->pass_len); malloc_check(userpass);
		userpass_len = db_params->pass_len;
		memcpy(userpass, db_params->pass, userpass_len);
	}

	while (yk) {
		/* If a YubiKey serial number was provided instead of a device
		 * index, try to lookup the device index from the serial
		 * number and store it in the yk list item's 'dev' member.
		 */
		if (yk->serial) {
			if (getenv("KC_DEBUG"))
				printf("%s(): searching for a device with serial: %d\n", __func__, yk->serial);

			yk->dev = -1;
			for (yk_counter = 0; 1; yk_counter++) {
				if (getenv("KC_DEBUG"))
					printf("%s(): trying to open device with index #%zu, slot = %d, dev = %d, serial = %d\n", __func__, yk_counter, yk->slot, yk->dev, yk->serial);
				if (!(yk_key = yk_open_key(yk_counter)))
					break;

				if (yk_get_serial(yk_key, 1, 0, &yk_serial)) {
					if (yk->serial == yk_serial)
						yk->dev = yk_counter;

					if (yk_key && !yk_close_key(yk_key)) {
						goto err;
					}
					yk_key = NULL;
				} else {
					dprintf(STDERR_FILENO, "ERROR: Could not read serial number from YubiKey device #%zu!\n", yk_counter);
					goto err;
				}
			}

			if (yk->dev < 0) {
				dprintf(STDERR_FILENO, "ERROR: Could not find YubiKey device with serial number %d!\n", yk->serial);
				yk_errno = YK_ENOKEY;
				goto err;
			}
		}

		yk = yk->next;
	}

	yk_errno = 0;
	yk_counter = 0;
	yk = db_params->yk;
	while (yk) {
		yk_counter++;

		printf("Using YubiKey slot #%d on device #%d\n", yk->slot, yk->dev);

		if (getenv("KC_DEBUG"))
			printf("%s(): yk_counter: %zu, yk array:\nyk->slot = %d\nyk->dev = %d\nyk->serial = %d\n", __func__, yk_counter, yk->slot, yk->dev, yk->serial);


		if (getenv("KC_DEBUG"))
			printf("%s(): trying to open device with index #%d\n", __func__, yk->dev);
		if (!(yk_key = yk_open_key((int)yk->dev))) {
			goto err;
		}

		if (getenv("KC_DEBUG"))
			printf("%s(): checking firmware version\n", __func__);
		if (!yk_check_firmware(yk_key)) {
			goto err;
		}

		if (userpass  &&  userpass_len > YUBIKEY_CHALLENGE_MAXLEN) {
			dprintf(STDERR_FILENO, "ERROR: Password cannot be longer than %d bytes when using YubiKey challenge-response!\n", YUBIKEY_CHALLENGE_MAXLEN);
			yk_errno = YK_EWRONGSIZ;
			goto err;
		}

		switch(yk->slot) {
			case 1:
				yk_cmd = SLOT_CHAL_HMAC1;
				break;
			case 2:
				yk_cmd = SLOT_CHAL_HMAC2;
				break;
			default:
				dprintf(STDERR_FILENO, "ERROR: YubiKey slot #%d is invalid!\n", yk->slot);
				yk_errno = YK_EINVALIDCMD;
				goto err;
		}


		/* set up challenge */
		if (!userpass  &&  yk_counter == 1) {
			/* Use the salt as the challenge when we *don't* want
			 * to use a user-password with the security key *and*
			 * this is the first key.
			 * The latter is important, because even if we don't
			 * use a user-password, we still re-use the response
			 * (that is saved in db->pass)  from previous keys when
			 * using multiple security keys.
			 * Otherwise, when using multiple security keys, even
			 * if we don't want to use a user-password, if this is
			 * not the very first key, then we'll re-use the
			 * previous response (in the other branch of the
			 * conditional, below).
			 */
			if (getenv("KC_DEBUG"))
				printf("%s(): automatic yubikey, without password\n", __func__);

			/* this won't be the digest of the salt, but the actual
			 * decoded salt */
			challenge_len = SALT_LEN < YUBIKEY_CHALLENGE_MAXLEN ? SALT_LEN : YUBIKEY_CHALLENGE_MAXLEN;
			challenge = malloc(challenge_len); malloc_check(challenge);
			yubikey_hex_decode((char *)challenge, (char *)db_params->salt, challenge_len);
			if (getenv("KC_DEBUG"))
				printf("%s(): the challenge is filled with %zu bytes from salt\n", __func__, challenge_len);
		} else {
			/* We're re-using the previous response (db->pass) when
			 * using multiple security keys to treat the password,
			 * even if there's no user-supplied password in
			 * db->pass. This makes for a kind of chained security
			 * key usage.
			 */
			if (getenv("KC_DEBUG"))
				printf("%s(): using password with yubikey\n", __func__);

			/* Here we not only copy the password to the challenge,
			 * but also append data from the salt if possible -- as
			 * much data as there is space left from
			 * YUBIKEY_CHALLENGE_MAXLEN after filling it with the
			 * password.
			 */
			challenge_len = db_params->pass_len + SALT_DIGEST_LEN < YUBIKEY_CHALLENGE_MAXLEN ? db_params->pass_len + SALT_DIGEST_LEN : YUBIKEY_CHALLENGE_MAXLEN;
			challenge = malloc(challenge_len); malloc_check(challenge);

			/* this is so we know how much we copied from db->pass
			 * to the challenge
			 */
			copied = challenge_len < db_params->pass_len ? challenge_len : db_params->pass_len;
			/* copy the password to the challenge first ... */
			memcpy(challenge, db_params->pass, copied);

			/* ... then append from the salt if there's any space left in 'challenge' */
			memcpy(challenge + copied, db_params->salt, challenge_len - copied);

			if (getenv("KC_DEBUG"))
				printf("%s(): the challenge(len:%zu) is filled with %zu bytes from salt after %zu bytes of password\n", __func__, challenge_len, challenge_len - db_params->pass_len, db_params->pass_len);
		}


		printf("Remember to touch your YubiKey if necessary\n");
		memset(response, '\0', sizeof(response));
		if (!yk_challenge_response(yk_key, yk_cmd, may_block,
			challenge_len, challenge,
			sizeof(response), response))
		{
			goto err;
		}

		memset(challenge, '\0', sizeof(challenge_len));
		free(challenge); challenge = NULL;

		if (yk_key && !yk_close_key(yk_key)) {
			goto err;
		}
		yk_key = NULL;

		/* realloc ..->pass and use the response */
		/* copy the response as the constructed password */
		memset(db_params->pass, '\0', db_params->pass_len);
		db_params->pass_len = sizeof(response) > PASSWORD_MAXLEN ? PASSWORD_MAXLEN : sizeof(response);
		db_params->pass = realloc(db_params->pass, db_params->pass_len); malloc_check(db_params->pass);
		memcpy(db_params->pass, response, db_params->pass_len);
		memset(response, '\0', sizeof(response));

		yk = yk->next;
	}

	/* If there's a user-password, then append it to the response, but only
	 * if this is the last security key (we're out of the loop here, where
	 * we've iterated the security keys) in the chain, if there are
	 * multiple ones provided.
	 * This is because YubiKey's challenge-response mode can handle a
	 * challenge that's at most 64 bytes long, and with this (appending the
	 * password) we're creating something that's longer than that.
	 */
	if (userpass) {
		if (getenv("KC_DEBUG"))
			printf("%s(): constructing new password by appending user-password to yubikey response\n", __func__);

		db_params->pass_len = db_params->pass_len + userpass_len > PASSWORD_MAXLEN ? PASSWORD_MAXLEN : db_params->pass_len + userpass_len;
		db_params->pass = realloc(db_params->pass, db_params->pass_len); malloc_check(db_params->pass);

		/* append the actual user-password as well to the end of the constructed password */
		memcpy(db_params->pass + sizeof(response), userpass, db_params->pass_len - sizeof(response));
	}

	if (getenv("KC_DEBUG"))
		printf("%s(): pass_len: %zu\n", __func__, db_params->pass_len);

err:
	if (userpass)
		memset(userpass, '\0', userpass_len);
	free(userpass); userpass = NULL;
	userpass_len = 0;

	if (challenge)
		memset(challenge, '\0', sizeof(challenge_len));
	free(challenge); challenge = NULL;
	challenge_len = 0;

	memset(response, '\0', sizeof(response));

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
				yk->dev, yk->slot, yk_strerror(yk_errno));
		}
	}
}/* yk_report_error() */
