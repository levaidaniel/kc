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

#include <yubikey.h>
#include <ykpers-1/ykcore.h>
#include <ykpers-1/ykdef.h>
/*
#include <ykpers-1/ykstatus.h>
*/

#include "common.h"
#include "ykchalresp.h"


#define	SHA1_MAX_BLOCK_SIZE	64	/* Max size of input SHA1 block */


int
kc_ykchalresp(struct db_parameters *db_params)
{
	YK_KEY *yk = 0;
	int yk_cmd = 0;

	bool may_block = true;
	unsigned char response[SHA1_MAX_BLOCK_SIZE];
	unsigned char output_buf[(SHA1_MAX_BLOCK_SIZE * 2) + 1];

	yk_errno = 0;


	if (!yk_init()) {
		report_yk_error();
		return(0);
	}

	if (!(yk = yk_open_key((int)db_params->ykdev))) {
		report_yk_error();
		return(0);
	}

/*
 * ykstatus.h is broken here with an #include <ykcore.h> line
	if (!check_firmware(yk)) {
		return(0);
	}
*/

	switch(db_params->ykslot) {
		case 1:
			yk_cmd = SLOT_CHAL_HMAC1;
			break;
		case 2:
			yk_cmd = SLOT_CHAL_HMAC2;
			break;
		default:
			return(0);
	}

	memset(response, 0, sizeof(response));
	memset(output_buf, 0, sizeof(output_buf));

	printf("Remember to touch your YubiKey if necessary\n");
	if(!yk_challenge_response(yk, yk_cmd, may_block,
		db_params->pass_len, (unsigned char *)db_params->pass,
		sizeof(response), response))
	{
		report_yk_error();
		return(0);
	}

	if (yk && !yk_close_key(yk)) {
		report_yk_error();
		return(0);
	}

	/* HMAC responses are 160 bits (i.e. 20 bytes) */
	yubikey_hex_encode((char *)output_buf, (char *)response, 20);
	memset(response, 0, sizeof(response));
	memset(db_params->pass, '\0', db_params->pass_len);
	memcpy(db_params->pass, output_buf, db_params->pass_len);


	return(1);
} /* kc_ykchalresp() */


/*
 * ykstatus.h is broken here with an #include <ykcore.h> line
static int check_firmware(YK_KEY *yk)
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
}*/ /* check_firmware() */


static void report_yk_error(void)
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
}
