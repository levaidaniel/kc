/*
 * Copyright (c) 2011-2013 LEVAI Daniel
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


#include "common.h"
#include "commands.h"


extern db_parameters	db_params;
extern BIO		*bio_chain;


void
cmd_passwd(const char *e_line, command *commands)
{
	char		ret = -1;
	char		*line = NULL, *kdf = NULL, *cipher_mode = NULL;


	/* ask for the new password */
	while (ret == -1)
		ret = kc_password_read(&db_params.pass, 1);

	if (ret == 0)	/* canceled */
		return;

	line = strdup(e_line);

	strtok(line, " ");			/* remove the command from the line */

	kdf = strtok(NULL, " ");		/* assign the command's first parameter (kdf) */
	/* Changed KDF */
	if (kdf) {
		free(db_params.kdf); db_params.kdf = NULL;
		db_params.kdf = strdup(kdf);
	}

	cipher_mode = strtok(NULL, " ");	/* assign the command's second parameter (cipher_mode) */
	/* Changed cipher mode */
	if (cipher_mode) {
		free(db_params.cipher_mode); db_params.cipher_mode = NULL;
		db_params.cipher_mode = strdup(cipher_mode);
	}

	free(line); line = NULL;

	ret = kc_setup_crypt(bio_chain, 1, &db_params, KC_SETUP_CRYPT_IV | KC_SETUP_CRYPT_SALT | KC_SETUP_CRYPT_KEY);

	if (db_params.pass)
		memset(db_params.pass, '\0', PASSWORD_MAXLEN);
	free(db_params.pass); db_params.pass = NULL;

	if (ret) {
		cmd_write(NULL, NULL);
		puts("Password change OK");
	} else
		puts("Could not change password!");
} /* cmd_passwd() */
