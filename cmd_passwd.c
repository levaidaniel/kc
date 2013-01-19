/*
 * Copyright (c) 2011, 2012, 2013 LEVAI Daniel
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
 * DISCLAIMED. IN NO EVENT SHALL LEVAI Daniel BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "common.h"
#include "commands.h"


extern BIO		*bio_chain;

extern char		*cipher_mode;

extern unsigned char	salt[17], iv[17], key[128];


void
cmd_passwd(const char *e_line, command *commands)
{
	char		*pass = NULL;
	char		ret = -1;


	/* ask for the new password */
	while (ret == -1)
		ret = kc_password_read(&pass, 1);

	if (ret == 0)	/* canceled */
		return;

	kc_setup_crypt(bio_chain, 1, cipher_mode, pass, iv, salt, key, KC_SETUP_CRYPT_IV | KC_SETUP_CRYPT_SALT | KC_SETUP_CRYPT_KEY);

	if (pass)
		memset(pass, '\0', PASSWORD_MAXLEN);
	free(pass); pass = NULL;

	cmd_write(NULL, NULL);
} /* cmd_passwd() */
