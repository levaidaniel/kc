/*
 * Copyright (c) 2011-2020 LEVAI Daniel
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
#include "ssha.h"
#ifdef _HAVE_YUBIKEY
#include "ykchalresp.h"
#endif


extern db_parameters	db_params;
extern BIO		*bio_chain;


void
cmd_passwd(const char *e_line, command *commands)
{
	int	c = 0, largc = 0;
	size_t	len = 0;
	char	*opts = NULL, *inv = NULL;
	char	**largv = NULL;
	char	*line = NULL;
	char	*ssha_type = NULL, *ssha_comment = NULL;

#ifdef _HAVE_YUBIKEY
	unsigned long int	ykchalresp = 0;
#endif

	db_parameters	db_params_tmp;


	/* initial db_params for the temporary database */
	db_params_tmp.ssha_type[0] = '\0';
	db_params_tmp.ssha_comment[0] = '\0';
	db_params_tmp.ssha_password = 0;
	db_params_tmp.yk_dev = 0;
	db_params_tmp.yk_slot = 0;
	db_params_tmp.yk_password = 0;
	db_params_tmp.pass = NULL;
	db_params_tmp.pass_len = 0;
	db_params_tmp.pass_filename = NULL;
	db_params_tmp.kdf = NULL;
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
	opts = "A:P:R:e:m:Y:";
#else
	opts = "A:P:R:e:m:";
#endif
	optind = 0;
	while ((c = getopt(largc, largv, opts)) != -1)
		switch (c) {
			case 'A':
				if (strlen(db_params_tmp.ssha_type)  ||  strlen(db_params_tmp.ssha_comment)) {
					dprintf(STDERR_FILENO, "ERROR: Please specify the '-%c' option only once!\n", c);
					goto exiting;
				}


				ssha_type = strndup(strsep(&optarg, ","), 11);
				if (ssha_type == NULL  ||  !strlen(ssha_type)) {
					dprintf(STDERR_FILENO, "ERROR: SSH key type is empty!\n");
					goto exiting;
				}
				if (	strncmp(ssha_type, "ssh-rsa", 7) != 0  &&
					strncmp(ssha_type, "ssh-ed25519", 11) != 0
				) {
					dprintf(STDERR_FILENO, "ERROR: SSH key type is unsupported: '%s'\n", ssha_type);
					goto exiting;
				}

				ssha_comment = strndup(strsep(&optarg, ","), 512);
				if (ssha_comment == NULL  ||  !strlen(ssha_comment)) {
					dprintf(STDERR_FILENO, "ERROR: SSH key comment is empty!\n");
					goto exiting;
				}

				if (strlcpy(db_params_tmp.ssha_type, ssha_type, sizeof(db_params_tmp.ssha_type)) >= sizeof(db_params_tmp.ssha_type)) {
					dprintf(STDERR_FILENO, "ERROR: Error while getting SSH key type.\n");
					goto exiting;
				}

				if (strlcpy(db_params_tmp.ssha_comment, ssha_comment, sizeof(db_params_tmp.ssha_comment)) >= sizeof(db_params_tmp.ssha_comment)) {
					dprintf(STDERR_FILENO, "ERROR: Error while getting SSH key comment.\n");
					goto exiting;
				}

				if (optarg  &&  strncmp(optarg, "password", 8) == 0) {
					db_params_tmp.ssha_password = 1;
				}
			break;
			case 'P':
				if (db_params_tmp.kdf) {
					dprintf(STDERR_FILENO, "ERROR: Please specify the '-%c' option only once!\n", c);
					goto exiting;
				}
				db_params_tmp.kdf = strdup(optarg);
			break;
			case 'R':
				if (db_params_tmp.kdf_reps) {
					dprintf(STDERR_FILENO, "ERROR: Please specify the '-%c' option only once!\n", c);
					goto exiting;
				}


				if (optarg[0] == '-') {
					dprintf(STDERR_FILENO, "ERROR: KDF iterations parameter seems to be negative.\n");
					goto exiting;
				}

				db_params_tmp.kdf_reps = strtoul(optarg, &inv, 10);
				if (inv[0] != '\0') {
					dprintf(STDERR_FILENO, "ERROR: Unable to convert the KDF iterations parameter.\n");
					goto exiting;
				}
			break;
			case 'e':
				if (db_params_tmp.cipher) {
					dprintf(STDERR_FILENO, "ERROR: Please specify the '-%c' option only once!\n", c);
					goto exiting;
				}
				db_params_tmp.cipher = strdup(optarg);
			break;
			case 'm':
				if (db_params_tmp.cipher_mode) {
					dprintf(STDERR_FILENO, "ERROR: Please specify the '-%c' option only once!\n", c);
					goto exiting;
				}
				db_params_tmp.cipher_mode = strdup(optarg);
			break;
#ifdef _HAVE_YUBIKEY
			case 'Y':
				if (db_params_tmp.yk_slot) {
					dprintf(STDERR_FILENO, "ERROR: Please specify the '-%c' option only once!\n", c);
					goto exiting;
				}


				if (optarg[0] == '-') {
					dprintf(STDERR_FILENO, "ERROR: YubiKey slot/device parameter seems to be negative.\n");
					goto exiting;
				}

				ykchalresp = strtoul(strsep(&optarg, ","), &inv, 10);
				if (inv[0] == '\0') {
					if (ykchalresp <= 0  ||  ykchalresp > 29) {
						dprintf(STDERR_FILENO, "ERROR: YubiKey slot/device parameter is invalid.\n");
						goto exiting;
					} else if (ykchalresp < 10) {
						db_params_tmp.yk_slot = ykchalresp;
						db_params_tmp.yk_dev = 0;
					} else {
						db_params_tmp.yk_slot = ykchalresp / 10 ;
						db_params_tmp.yk_dev = ykchalresp - (ykchalresp / 10 * 10);
					}
				} else {
					dprintf(STDERR_FILENO, "ERROR: Unable to convert the YubiKey slot/device parameter.\n");
					goto exiting;
				}

				if (db_params_tmp.yk_slot > 2  ||  db_params_tmp.yk_slot < 1) {
					dprintf(STDERR_FILENO, "ERROR: YubiKey slot number is not 1 or 2.\n");
					goto exiting;
				}

				if (optarg  &&  strncmp(strsep(&optarg, ","), "password", 8) == 0) {
					db_params_tmp.yk_password = 1;
				}
			break;
#endif
			default:
				puts(commands->usage);
				goto exiting;
			break;
		}

	/* clean up after option parsing */
	free(ssha_type); ssha_type = NULL;
	free(ssha_comment); ssha_comment = NULL;

	/* print some status information after parsing the options */
	if (	(strlen(db_params_tmp.ssha_type)  &&  db_params_tmp.yk_slot)  &&
		(!db_params_tmp.ssha_password  ||  !db_params_tmp.yk_password)
	) {
		dprintf(STDERR_FILENO, "ERROR: Using -A and -Y together only makes sense with the ',password' parameter for both of them!\n");
		goto exiting;
	}

	if (strlen(db_params_tmp.ssha_type))
		printf("Using (%s) %s identity%s\n", db_params_tmp.ssha_type, db_params_tmp.ssha_comment, (db_params_tmp.ssha_password ? " and a password" : ""));
	if (db_params_tmp.yk_slot)
		printf("Using YubiKey slot #%d on device #%d%s\n", db_params_tmp.yk_slot, db_params_tmp.yk_dev, (db_params_tmp.yk_password ? " and a password" : ""));


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
			quit(EXIT_FAILURE);
		}
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

	if (	db_params_tmp.ssha_password  ||
		db_params_tmp.yk_password  ||
		(!db_params_tmp.yk_slot  &&  !strlen(db_params_tmp.ssha_type))
	) {
		if (getenv("KC_DEBUG"))
			printf("%s(): getting a password for the database\n", __func__);

		/* ask for the new password */
		if (kc_password_read(&db_params_tmp, 1) != 1)
			goto exiting;
	}

#ifdef _HAVE_YUBIKEY
	if (db_params_tmp.yk_slot) {
		/* use a YubiKey to generate the password */
		if (!kc_ykchalresp(&db_params_tmp)) {
			dprintf(STDERR_FILENO, "ERROR: Error while doing YubiKey challenge-response!\n");
			goto exiting;
		}
	}
#endif
	if (strlen(db_params_tmp.ssha_type)) {
		/* use SSH agent to generate the password */
		if (!kc_ssha_get_password(&db_params_tmp))
			goto exiting;
	}

	/* Setup cipher mode and turn on encrypting */
	if (	kc_crypt_key(&db_params_tmp) != 1  ||
		kc_crypt_setup(bio_chain, 1, &db_params_tmp) != 1
	) {
		dprintf(STDERR_FILENO, "ERROR: Could not setup encrypting!\n");
		goto exiting;
	}


	/* store the new key, IV and salt in our working copy of 'db_params' */
	memcpy(db_params.key, db_params_tmp.key, KEY_LEN);
	if (memcmp(db_params.key, db_params_tmp.key, KEY_LEN) != 0) {
		dprintf(STDERR_FILENO, "ERROR: Could not copy encryption key!");
		goto exiting;
	}
	memset(db_params_tmp.key, '\0', KEY_LEN);
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

	db_params.yk_slot = db_params_tmp.yk_slot;
	db_params.yk_dev = db_params_tmp.yk_dev;
	db_params.yk_password = db_params_tmp.yk_password;

	db_params.pass_len = db_params_tmp.pass_len;

	cmd_status(NULL, NULL);
	cmd_write(NULL, NULL);
	puts("Password change OK");

exiting:
	for (c = 0; c < largc; c++) {
		free(largv[c]); largv[c] = NULL;
	}
	free(largv); largv = NULL;

	free(ssha_type); ssha_type = NULL;
	free(ssha_comment); ssha_comment = NULL;

	memset(db_params_tmp.key, '\0', KEY_LEN);
	if (db_params_tmp.pass)
		memset(db_params_tmp.pass, '\0', db_params_tmp.pass_len);
	free(db_params_tmp.pass); db_params_tmp.pass = NULL;

	free(db_params_tmp.kdf); db_params_tmp.kdf = NULL;
	free(db_params_tmp.cipher); db_params_tmp.cipher = NULL;
	free(db_params_tmp.cipher_mode); db_params_tmp.cipher_mode = NULL;
} /* cmd_passwd() */
