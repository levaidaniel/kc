/*
 * Copyright (c) 2011-2023 LEVAI Daniel
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
	size_t	len = 0;
	char	*opts = NULL;
	char	**largv = NULL;
	char	*line = NULL;

	extra_parameters	params;

	db_parameters	db_params_tmp;


	params.caller = "passwd";

	/* initial db_params for the temporary database */
	db_params_tmp.ssha_type[0] = '\0';
	db_params_tmp.ssha_comment[0] = '\0';
	db_params_tmp.ssha_password = 0;
	db_params_tmp.yk = NULL;
	db_params_tmp.yk_password = 0;
	db_params_tmp.pass = NULL;
	db_params_tmp.pass_len = 0;
	db_params_tmp.pass_filename = NULL;
	db_params_tmp.kdf = NULL;
	db_params_tmp.key_len = 0;
	db_params_tmp.key = NULL;
	db_params_tmp.kdf_reps = 0;
	db_params_tmp.cipher = NULL;
	db_params_tmp.cipher_mode = NULL;


	/* Parse the arguments */
	line = strdup(e_line);
	if (!line) {
		perror("ERROR: Could not duplicate the command line");
		goto exiting;
	}
	larg(line, &largv, &largc);
	free(line); line = NULL;

#ifdef _HAVE_YUBIKEY
	opts = "A:P:K:R:e:m:Y:";
#else
	opts = "A:P:K:R:e:m:";
#endif
	switch (kc_arg_parser(largc, largv, opts, &db_params_tmp, &params)) {
		case -1:
			goto exiting;
		break;
		case 0:
			puts(commands->usage);
			goto exiting;
		break;
		case 1:
			if (getenv("KC_DEBUG"))
				printf("%s(): Parameter parsing is successful.\n", __func__);
		break;
	}


	/* use original KDF, if none was specified */
	if (!db_params_tmp.kdf) {
		len = strlen(db_params.kdf) + 1;
		db_params_tmp.kdf = malloc(len); malloc_check(db_params_tmp.kdf);
		if (strlcpy(db_params_tmp.kdf, db_params.kdf, len) >= len) {
			dprintf(STDERR_FILENO, "ERROR: Error while setting up database parameters (kdf).\n");
			goto exiting;
		}
	}

	/* reset kdf reps only if kdf was changed and no -R option was
	 * specified */
	if (!db_params_tmp.kdf_reps) {
		/* -R option was not specified, because our default is 0 */
		if (strcmp(db_params.kdf, db_params_tmp.kdf) == 0) {
			db_params_tmp.kdf_reps = db_params.kdf_reps;
		} else {
			if (strncmp(db_params_tmp.kdf, "sha", 3) == 0) {
				db_params_tmp.kdf_reps = KC_PKCS_PBKDF2_ITERATIONS;
			} else if (strcmp(db_params_tmp.kdf, "bcrypt") == 0) {
				db_params_tmp.kdf_reps = KC_BCRYPT_PBKDF_ROUNDS;
			}
		}
	}
	if (strncmp(db_params_tmp.kdf, "sha", 3) == 0  &&  db_params_tmp.kdf_reps < 1000) {
		dprintf(STDERR_FILENO, "ERROR: When using %s KDF, iterations (-R option) should be at least 1000 (the default is %d)\n", db_params_tmp.kdf, KC_PKCS_PBKDF2_ITERATIONS);
		goto exiting;
	} else if (strcmp(db_params_tmp.kdf, "bcrypt") == 0  &&  db_params_tmp.kdf_reps < 16) {
		dprintf(STDERR_FILENO, "ERROR: When using %s KDF, iterations (-R option) should be at least 16 (the default is %d)\n", db_params_tmp.kdf, KC_BCRYPT_PBKDF_ROUNDS);
		goto exiting;
	}

	/* use original encryption cipher, if none was specified */
	if (!db_params_tmp.cipher) {
		len = strlen(db_params.cipher) + 1;
		db_params_tmp.cipher = malloc(len); malloc_check(db_params_tmp.cipher);
		if (strlcpy(db_params_tmp.cipher, db_params.cipher, len) >= len) {
			dprintf(STDERR_FILENO, "ERROR: Error while setting up database parameters (cipher).\n");
			goto exiting;
		}
	}

	/* reset key length only if cipher was changed and no -K option was
	 * specified.
	 * This needs to come after we figured out our cipher */
	if (!db_params_tmp.key_len) {
		db_params_tmp.key_len = KEY_MAX_LEN;

		/* -K option was not specified, because our default is 0 */
		if (strcmp(db_params.cipher, db_params_tmp.cipher) == 0) {
			db_params_tmp.key_len = db_params.key_len;
		}
	}
	if (	strncmp(db_params_tmp.cipher, "aes256", 6) == 0  &&
		db_params_tmp.key_len < KEY_MAX_LEN) {
			printf("WARNING: Resetting encryption key length to %d!\n", KEY_MAX_LEN);
			db_params_tmp.key_len = KEY_MAX_LEN;
	}

	/* reset cipher mode only if cipher was changed and no -m option was
	 * specified */
	if (!db_params_tmp.cipher_mode) {
		/* -m option was not specified, because our default is NULL */
		if (strcmp(db_params.cipher, db_params_tmp.cipher) == 0) {
			db_params_tmp.cipher_mode = strdup(db_params.cipher_mode);
			if (!db_params_tmp.cipher_mode) {
				perror("ERROR: Could not duplicate the cipher mode");
				goto exiting;
			}
		} else {
			len = strlen(DEFAULT_MODE) + 1;
			db_params_tmp.cipher_mode = malloc(len); malloc_check(db_params_tmp.cipher_mode);
			if (strlcpy(db_params_tmp.cipher_mode, DEFAULT_MODE, len) >= len) {
				dprintf(STDERR_FILENO, "ERROR: Error while setting up default database parameters (cipher mode).\n");
				goto exiting;
			}
		}
	}


	if (kc_crypt_iv_salt(&db_params_tmp) != 1) {
		dprintf(STDERR_FILENO, "ERROR: Could not generate IV and/or salt!\n");
		goto exiting;
	}

	/* Get a password into the database */
	if (kc_crypt_pass(&db_params_tmp, 1) != 1) {
			dprintf(STDERR_FILENO, "ERROR: Could not get a password!\n");
			goto exiting;
	}

	puts("Encrypting...");

	/* Setup cipher mode and turn on encrypting */
	if (	kc_crypt_key(&db_params_tmp) != 1  ||
		kc_crypt_setup(bio_chain, 1, &db_params_tmp) != 1
	) {
		dprintf(STDERR_FILENO, "ERROR: Could not setup encrypting!\n");
		goto exiting;
	}


	/* store the new key, IV and salt in our working copy of 'db_params' */

	/* we re-allocate the 'key' space for perhaps the key length has been changed */
	if (db_params.key)
		memset(db_params.key, '\0', db_params.key_len);
	free(db_params.key); db_params.key = NULL;
	db_params.key = malloc(db_params_tmp.key_len); malloc_check(db_params.key);

	db_params.key_len = db_params_tmp.key_len;
	memcpy(db_params.key, db_params_tmp.key, db_params_tmp.key_len);
	if (memcmp(db_params.key, db_params_tmp.key, db_params_tmp.key_len) != 0) {
		dprintf(STDERR_FILENO, "ERROR: Could not copy encryption key!");
		goto exiting;
	}
	if (db_params_tmp.key)
		memset(db_params_tmp.key, '\0', db_params_tmp.key_len);
	free(db_params_tmp.key); db_params_tmp.key = NULL;
	if (db_params_tmp.pass)
		memset(db_params_tmp.pass, '\0', db_params_tmp.pass_len);
	free(db_params_tmp.pass); db_params_tmp.pass = NULL;

	if (strlcpy((char *)db_params.iv, (const char*)db_params_tmp.iv, sizeof(db_params.iv)) >= sizeof(db_params.iv)) {
		dprintf(STDERR_FILENO, "ERROR: Could not copy IV!\n");
		goto exiting;
	}
	if (strlcpy((char *)db_params.salt, (const char*)db_params_tmp.salt, sizeof(db_params.salt)) >= sizeof(db_params.salt)) {
		dprintf(STDERR_FILENO, "ERROR: Could not copy salt!\n");
		goto exiting;
	}

	free(db_params.kdf); db_params.kdf = NULL;
	db_params.kdf = strdup(db_params_tmp.kdf);
	if (!db_params.kdf) {
		perror("ERROR: Could not save the KDF");
		goto exiting;
	}
	db_params.kdf_reps = db_params_tmp.kdf_reps;
	free(db_params.cipher); db_params.cipher = NULL;
	db_params.cipher = strdup(db_params_tmp.cipher);
	if (!db_params.cipher) {
		perror("ERROR: Could not save the cipher");
		goto exiting;
	}
	free(db_params.cipher_mode); db_params.cipher_mode = NULL;
	db_params.cipher_mode = strdup(db_params_tmp.cipher_mode);
	if (!db_params.cipher_mode) {
		perror("ERROR: Could not save the cipher mode");
		goto exiting;
	}
	if (strlcpy((char *)db_params.ssha_type, (const char*)db_params_tmp.ssha_type, sizeof(db_params.ssha_type)) >= sizeof(db_params.ssha_type)) {
		dprintf(STDERR_FILENO, "ERROR: Could not save SSH key type!\n");
		goto exiting;
	}
	if (strlcpy((char *)db_params.ssha_comment, (const char*)db_params_tmp.ssha_comment, sizeof(db_params.ssha_comment)) >= sizeof(db_params.ssha_comment)) {
		dprintf(STDERR_FILENO, "ERROR: Could not save SSH key comment!\n");
		goto exiting;
	}
	db_params.ssha_password = db_params_tmp.ssha_password;

	db_params.yk = db_params_tmp.yk;

	db_params.pass_len = db_params_tmp.pass_len;

	cmd_status(NULL, NULL);
	cmd_write(NULL, NULL);
	puts("Password change OK");

exiting:
	for (c = 0; c < largc; c++) {
		free(largv[c]); largv[c] = NULL;
	}
	free(largv); largv = NULL;

	if (db_params_tmp.key)
		memset(db_params_tmp.key, '\0', db_params_tmp.key_len);
	free(db_params_tmp.key); db_params_tmp.key = NULL;
	if (db_params_tmp.pass)
		memset(db_params_tmp.pass, '\0', db_params_tmp.pass_len);
	free(db_params_tmp.pass); db_params_tmp.pass = NULL;

	free(db_params_tmp.kdf); db_params_tmp.kdf = NULL;
	free(db_params_tmp.cipher); db_params_tmp.cipher = NULL;
	free(db_params_tmp.cipher_mode); db_params_tmp.cipher_mode = NULL;
} /* cmd_passwd() */
