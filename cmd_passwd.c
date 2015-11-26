/*
 * Copyright (c) 2011-2014 LEVAI Daniel
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
	int	c = 0, largc = 0;
	char	**largv = NULL;
	char	ret = -1, exiting = 0;
	char	*line = NULL;


	/* ask for the new password */
	while (ret == -1)
		ret = kc_password_read(&db_params.pass, 1);

	if (ret == 0)	/* canceled */
		return;


	/* Parse the arguments */
	line = strdup(e_line); malloc_check(line);
	larg(line, &largv, &largc);
	free(line); line = NULL;

	optind = 1;
	while ((c = getopt(largc, largv, "P:")) != -1)
		switch (c) {
			case 'P':
				free(db_params.kdf); db_params.kdf = NULL;
				db_params.kdf = strdup(optarg); malloc_check(db_params.kdf);
			break;
			default:
				exiting++;
			break;
		}

	for (c = 0; c <= largc; c++) {
		free(largv[c]); largv[c] = NULL;
	}
	free(largv); largv = NULL;

	if (exiting)
		return;

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
