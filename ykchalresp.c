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


#define	RESPONSE_SIZE	20	/* HMAC responses are 160 bits (i.e. 20 bytes) */
#define	SHA1_MAX_BLOCK_SIZE	64	/* Max size of input SHA1 block */


static int yk_check_firmware(YK_KEY *yk);
static void yk_report_error(void);


int
kc_ykchalresp(struct db_parameters *db_params)
{
	YK_KEY *yk = 0;
	int yk_cmd = 0;

	bool may_block = true;

	unsigned char response[SHA1_MAX_BLOCK_SIZE];
	unsigned char output_buf[(SHA1_MAX_BLOCK_SIZE * 2) + 1];

	unsigned char *challenge = NULL;
	unsigned int challenge_len = 0;

	yk_errno = 0;


	if (!yk_init()) {
		goto err;
	}

	if (!(yk = yk_open_key((int)db_params->yk_dev))) {
		goto err;
	}

	if (!yk_check_firmware(yk)) {
		goto err;
	}

	switch(db_params->yk_slot) {
		case 1:
			yk_cmd = SLOT_CHAL_HMAC1;
			break;
		case 2:
			yk_cmd = SLOT_CHAL_HMAC2;
			break;
		default:
			goto err;
	}

	memset(response, 0, sizeof(response));
	memset(output_buf, 0, sizeof(output_buf));

	/* set up challenge */
	if (db_params->yk_password) {
		if (getenv("KC_DEBUG"))
			printf("%s(): using password with yubikey\n", __func__);

		challenge = malloc(db_params->pass_len); malloc_check(challenge);
		memcpy(challenge, db_params->pass, db_params->pass_len);
		challenge_len = db_params->pass_len;
	} else {
		if (getenv("KC_DEBUG"))
			printf("%s(): automatic yubikey, without password\n", __func__);

		/* this won't be the digest of the salt, but the actual decoded
		 * salt */
		challenge_len = SALT_LEN;
		challenge = malloc(challenge_len); malloc_check(challenge);
		yubikey_hex_decode((char *)challenge, (char *)db_params->salt, challenge_len);
	}

	printf("Remember to touch your YubiKey if necessary\n");
	if (!yk_challenge_response(yk, yk_cmd, may_block,
		challenge_len, challenge,
		sizeof(response), response))
	{
		goto err;
	}

	yubikey_hex_encode((char *)output_buf, (char *)response, 20);
	memset(response, 0, RESPONSE_SIZE);

	memset(db_params->pass, '\0', db_params->pass_len);
	free(db_params->pass); db_params->pass = NULL;
	db_params->pass_len = 0;

	if (db_params->yk_password) {
		if (getenv("KC_DEBUG"))
			printf("%s(): constructing new password by appending password to yubikey response\n", __func__);

		db_params->pass_len = RESPONSE_SIZE * 2 + challenge_len;
		db_params->pass = malloc(db_params->pass_len); malloc_check(db_params->pass);

		/* copy the response first as the beginning of the constructed password */
		memcpy(db_params->pass, output_buf, RESPONSE_SIZE * 2);
		/* append the actual user password as well to the end of the constructed password
		 * ^^^ that is now in the 'challenge' variable -- we copied it above */
		memcpy(db_params->pass + RESPONSE_SIZE * 2, challenge, challenge_len);
	} else {
		if (getenv("KC_DEBUG"))
			printf("%s(): constructing new password by appending salt digest to yubikey response\n", __func__);

		db_params->pass_len = RESPONSE_SIZE * 2 + SALT_DIGEST_LEN;
		db_params->pass = malloc(db_params->pass_len); malloc_check(db_params->pass);

		memcpy(db_params->pass, output_buf, RESPONSE_SIZE * 2);
		memcpy(db_params->pass + RESPONSE_SIZE * 2, db_params->salt, SALT_DIGEST_LEN);

	}

err:
	yk_report_error();
	if (yk && !yk_close_key(yk)) {
		yk_report_error();
	}

	if (yk_errno) {
		return(0);
	} else {
		return(1);
	}
} /* kc_ykchalresp() */


static int
yk_check_firmware(YK_KEY *yk)
{
	YK_STATUS *st = ykds_alloc();

	if (!yk_get_status(yk, st)) {
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
yk_report_error(void)
{
	if (yk_errno) {
		if (yk_errno == YK_EUSBERR) {
			fprintf(stderr, "USB error: %s\n",
				yk_usb_strerror());
		} else {
			fprintf(stderr, "YubiKey core error: %s\n",
				yk_strerror(yk_errno));
		}
	}
}/* yk_report_error */
