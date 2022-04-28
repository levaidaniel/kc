/*
 * Copyright (c) 2011-2022 LEVAI Daniel
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

#ifdef BSD
#include <fcntl.h>
#endif

#if defined(__linux__)  ||  defined(__CYGWIN__)
#include <sys/file.h>
#endif


#ifndef _READLINE
extern EditLine		*e;
extern History		*eh;
#endif

extern db_parameters	db_params;
extern xmlDocPtr	db;


void
cmd_export(const char *e_line, command *commands)
{
	BIO		*bio_chain = NULL;

	xmlDocPtr	db_tmp = NULL;
	xmlNodePtr	keychain = NULL, keychain_tmp = NULL, root_node_tmp = NULL;

	extra_parameters	params;

	db_parameters	db_params_new;

	int		c = 0, largc = 0;
	size_t		len = 0;
	char		*opts = NULL;
	char		**largv = NULL;
	char		*line = NULL;
	char		dump = 0;
	struct stat	st;

#ifndef _READLINE
	int		e_count = 0;
#endif


	params.caller = "export";
	params.cname = NULL;

	/* initial db_params for the exported database */
	db_params_new.ssha_type[0] = '\0';
	db_params_new.ssha_comment[0] = '\0';
	db_params_new.ssha_password = 0;
	db_params_new.yk = NULL;
	db_params_new.yk_password = 0;
	db_params_new.pass = NULL;
	db_params_new.pass_len = 0;
	db_params_new.db_file = -1;
	db_params_new.db_filename = NULL;
	db_params_new.pass_filename = NULL;
	db_params_new.kdf = NULL;
	db_params_new.kdf_reps = 0;
	db_params_new.cipher = NULL;
	db_params_new.cipher_mode = NULL;
	db_params_new.dirty = 0;
	db_params_new.readonly = 0;


	/* Parse the arguments */
	line = strdup(e_line);
	if (!line) {
		perror("ERROR: Could not duplicate the command line");
		goto exiting;
	}
	larg(line, &largv, &largc);
	free(line); line = NULL;

#ifdef _HAVE_YUBIKEY
	opts = "A:k:c:P:R:e:m:Y:";
#else
	opts = "A:k:c:P:R:e:m:";
#endif
	switch (kc_arg_parser(largc, largv, opts, &db_params_new, &params)) {
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


	/* db_param_new defaults, if none were specified */
	if (!db_params_new.kdf) {
		len = strlen(DEFAULT_KDF) + 1;
		db_params_new.kdf = malloc(len); malloc_check(db_params_new.kdf);
		if (strlcpy(db_params_new.kdf, DEFAULT_KDF, len) >= len) {
			dprintf(STDERR_FILENO, "ERROR: Error while setting up default database parameters (kdf).\n");
			goto exiting;
		}
	}

	/* reset kdf reps only if kdf was changed and no -R option was
	 * specified */
	if (!db_params_new.kdf_reps) {
		/* -R option was not specified, because our default is 0 */
		if (strcmp(db_params.kdf, db_params_new.kdf) == 0) {
			db_params_new.kdf_reps = db_params.kdf_reps;
		} else {
			if (strncmp(db_params_new.kdf, "sha", 3) == 0) {
				db_params_new.kdf_reps = KC_PKCS_PBKDF2_ITERATIONS;
			} else if (strcmp(db_params_new.kdf, "bcrypt") == 0) {
				db_params_new.kdf_reps = KC_BCRYPT_PBKDF_ROUNDS;
			}
		}
	}
	if (strncmp(db_params_new.kdf, "sha", 3) == 0  &&  db_params_new.kdf_reps < 1000) {
		dprintf(STDERR_FILENO, "ERROR: When using %s KDF, iterations (-R option) should be at least 1000 (the default is %d)\n", db_params_new.kdf, KC_PKCS_PBKDF2_ITERATIONS);
		goto exiting;
	} else if (strcmp(db_params_new.kdf, "bcrypt") == 0  &&  db_params_new.kdf_reps < 16) {
		dprintf(STDERR_FILENO, "ERROR: When using %s KDF, iterations (-R option) should be at least 16 (the default is %d)\n", db_params_new.kdf, KC_BCRYPT_PBKDF_ROUNDS);
		goto exiting;
	}

	if (!db_params_new.cipher) {
		len = strlen(DEFAULT_CIPHER) + 1;
		db_params_new.cipher = malloc(len); malloc_check(db_params_new.cipher);
		if (strlcpy(db_params_new.cipher, DEFAULT_CIPHER, len) >= len) {
			dprintf(STDERR_FILENO, "ERROR: Error while setting up default database parameters (cipher).\n");
			goto exiting;
		}
	}

	/* reset cipher mode only if cipher was changed and no -m option was
	 * specified */
	if (!db_params_new.cipher_mode) {
		/* -m option was not specified, because our default is NULL */
		if (strcmp(db_params.cipher, db_params_new.cipher) == 0) {
			db_params_new.cipher_mode = strdup(db_params.cipher_mode);
			if (!db_params_new.cipher_mode) {
				perror("ERROR: Could not duplicate the cipher mode");
				goto exiting;
			}
		} else {
			len = strlen(DEFAULT_MODE) + 1;
			db_params_new.cipher_mode = malloc(len); malloc_check(db_params_new.cipher_mode);
			if (strlcpy(db_params_new.cipher_mode, DEFAULT_MODE, len) >= len) {
				dprintf(STDERR_FILENO, "ERROR: Error while setting up default database parameters (cipher mode).\n");
				goto exiting;
			}
		}
	}


	if (strcmp(largv[0], "dump") == 0)
		dump = 1;


	if (!db_params_new.db_filename) {
		puts(commands->usage);
		goto exiting;
	}


	if (params.cname) {
		/* A 'keychain' was specified, so export only that one.
		 * We must create a new xmlDoc and copy the specified keychain to it.
		 */

		keychain = find_keychain(params.cname, 0);
		if (!keychain) {
			printf("'%s' keychain not found.\n", params.cname);

			goto exiting;
		}

		/* create the new document */
		db_tmp = xmlNewDoc(BAD_CAST "1.0");
		if (!db_tmp) {
			dprintf(STDERR_FILENO, "ERROR: Could not create the new XML document for export!\n");

			goto exiting;
		}

		/* A new root node */
		root_node_tmp = xmlNewNode(NULL, BAD_CAST "kc");
		if (!root_node_tmp) {
			dprintf(STDERR_FILENO, "ERROR: Could not create the new root node for export!\n");

			goto exiting;
		}
		xmlDocSetRootElement(db_tmp, root_node_tmp);

		/* add the specified keychain's node to the export xmlDoc */
		xmlAddChild(root_node_tmp, xmlNewText(BAD_CAST "\n\t"));
		keychain_tmp = xmlCopyNode(keychain, 1);
		if (!keychain_tmp) {
			dprintf(STDERR_FILENO, "ERROR: Could not duplicate keychain for export!\n");

			goto exiting;
		}
		xmlAddChild(root_node_tmp, keychain_tmp);

		xmlAddChild(root_node_tmp, xmlNewText(BAD_CAST "\n"));
	} else
		/* save the whole document */
		db_tmp = db;

	/* Check if the supplied filename contains an extension */
	if (!strchr(db_params_new.db_filename, '.')) {
		db_params_new.db_filename = realloc(db_params_new.db_filename,
							strlen(db_params_new.db_filename) + 4 + 1); malloc_check(db_params_new.db_filename);
		if (strlcat(	db_params_new.db_filename,
				dump ? ".xml" : ".kcd",
				strlen(db_params_new.db_filename) + 4 + 1)
				>= strlen(db_params_new.db_filename) + 4 + 1)
		{
			dprintf(STDERR_FILENO, "ERROR: Could not construct the name of the output file!\n");

			goto exiting;
		}
	}

	if (lstat(db_params_new.db_filename, &st) == 0) {	/* if export filename exists */
		printf("Do you want to overwrite '%s'? <yes/no> ", db_params_new.db_filename);

#ifndef _READLINE
		/* disable history temporarily */
		if (el_set(e, EL_HIST, history, NULL) != 0) {
			perror("ERROR: el_set(EL_HIST)");
		}
		/* clear the prompt temporarily */
		if (el_set(e, EL_PROMPT, el_prompt_null) != 0) {
			perror("ERROR: el_set(EL_PROMPT)");
		}

		e_line = el_gets(e, &e_count);

		/* re-enable the default prompt */
		if (el_set(e, EL_PROMPT, prompt_str) != 0) {
			perror("ERROR: el_set(EL_PROMPT)");
		}
		/* re-enable history */
		if (el_set(e, EL_HIST, history, eh) != 0) {
			perror("ERROR: el_set(EL_HIST)");
		}
#else
		rl_redisplay();
		e_line = readline("");
#endif

		if (!e_line) {
#ifndef _READLINE
			el_reset(e);
#endif
			goto exiting;
		}

		if (strncmp(e_line, "yes", 3) != 0) {
#ifdef _READLINE
			free((char *)e_line); e_line = NULL;
#endif
			goto exiting;
		}
	}

	if (dump) {
		if (xmlSaveFormatFileEnc(db_params_new.db_filename, db_tmp, "UTF-8", XML_SAVE_FORMAT) > 0) {
			if (chmod(db_params_new.db_filename, S_IRUSR | S_IWUSR) < 0)
				dprintf(STDERR_FILENO, "ERROR: Could not change permissions of dump file!\n");

			puts("Dump OK");
		} else
			dprintf(STDERR_FILENO, "ERROR: Failed dumping to '%s'.\n", db_params_new.db_filename);
	} else {
		db_params_new.db_file = open(db_params_new.db_filename, O_RDWR | O_CREAT, 0600);
		if (db_params_new.db_file < 0) {
			perror("ERROR: open(create database file)");
			goto exiting;
		}

		bio_chain = kc_setup_bio_chain(db_params_new.db_filename, 1);
		if (!bio_chain) {
			dprintf(STDERR_FILENO, "ERROR: Could not setup bio_chain!\n");
			goto exiting;
		}


		/* Generate iv/salt */
		if (kc_crypt_iv_salt(&db_params_new) != 1) {
			dprintf(STDERR_FILENO, "ERROR: Could not generate IV and/or salt!\n");
			goto exiting;
		}

		/* Get a password into the database */
		if (kc_crypt_pass(&db_params_new, 1) != 1) {
				dprintf(STDERR_FILENO, "ERROR: Could not get a password!\n");
				goto exiting;
		}

		puts("Encrypting...");

		/* Setup cipher mode and turn on encrypting */
		if (	kc_crypt_key(&db_params_new) != 1  ||
			kc_crypt_setup(bio_chain, 1, &db_params_new) != 1
		) {
			dprintf(STDERR_FILENO, "ERROR: Could not setup encrypting!\n");
			goto exiting;
		}
		memset(db_params_new.key, '\0', KEY_LEN);
		if (db_params_new.pass)
			memset(db_params_new.pass, '\0', db_params_new.pass_len);
		free(db_params_new.pass); db_params_new.pass = NULL;


		if (kc_db_writer(db_tmp, bio_chain, &db_params_new))
			puts("Export OK");
		else
			dprintf(STDERR_FILENO, "ERROR: Failed exporting to '%s'!\n", db_params_new.db_filename);

	}


exiting:
	for (c = 0; c < largc; c++) {
		free(largv[c]); largv[c] = NULL;
	}
	free(largv); largv = NULL;

	if (bio_chain) {
		BIO_free_all(bio_chain);
		bio_chain = NULL;
	}

	if (db_params_new.db_file >= 0)
		close(db_params_new.db_file);

	if (params.cname  &&  db_tmp)
		xmlFreeDoc(db_tmp);	/* if we saved a specific keychain, clean up the temporary xmlDoc and its tree. */

	free(params.cname); params.cname = NULL;

	memset(db_params_new.key, '\0', KEY_LEN);
	if (db_params_new.pass)
		memset(db_params_new.pass, '\0', db_params_new.pass_len);
	free(db_params_new.pass); db_params_new.pass = NULL;

	free(db_params_new.kdf); db_params_new.kdf = NULL;
	free(db_params_new.cipher); db_params_new.cipher = NULL;
	free(db_params_new.cipher_mode); db_params_new.cipher_mode = NULL;
	free(db_params_new.db_filename); db_params_new.db_filename = NULL;
} /* cmd_export() */
