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

#ifdef BSD
#include <fcntl.h>
#endif

#ifdef __linux__
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
	xmlChar		*cname = NULL;

	db_parameters	db_params_new;

	int		c = 0, largc = 0;
	char		**largv = NULL;
	char		*line = NULL;
	char		dump = 0, ret = -1;
	struct stat	st;

#ifndef _READLINE
	int		e_count = 0;
#endif


	/* initial db_params for the exported database */
	db_params_new.pass = NULL;
	db_params_new.db_filename = NULL;
	db_params_new.db_file = -1;
	db_params_new.pass_filename = NULL;
	db_params_new.kdf = strdup(db_params.kdf);
	db_params_new.cipher = strdup(db_params.cipher);
	db_params_new.cipher_mode = strdup(db_params.cipher_mode);
	db_params_new.dirty = 0;
	db_params_new.readonly = 0;


	/* Parse the arguments */
	line = strdup(e_line);
	larg(line, &largv, &largc);
	free(line); line = NULL;

	optind = 1;
	while ((c = getopt(largc, largv, "k:c:P:e:m:")) != -1)
		switch (c) {
			case 'k':
				db_params_new.db_filename = strdup(optarg);
			break;
			case 'c':
				cname = BAD_CAST strdup(optarg);
			break;
			case 'P':
				free(db_params_new.kdf); db_params_new.kdf = NULL;
				db_params_new.kdf = strdup(optarg);
			break;
			case 'e':
				free(db_params_new.cipher); db_params_new.cipher = NULL;
				db_params_new.cipher = strdup(optarg);
			break;
			case 'm':
				free(db_params_new.cipher_mode); db_params_new.cipher_mode = NULL;
				db_params_new.cipher_mode = strdup(optarg);
			break;
		}

	if (strcmp(largv[0], "dump") == 0)
		dump = 1;


	for (c = 0; c <= largc; c++) {
		free(largv[c]); largv[c] = NULL;
	}
	free(largv); largv = NULL;

	if (!db_params_new.db_filename) {
		puts(commands->usage);

		goto exiting;
	}


	if (cname) {
		/* A 'keychain' was specified, so export only that one.
		 * We must create a new xmlDoc and copy the specified keychain to it.
		 */

		keychain = find_keychain(cname, 0);
		if (!keychain) {
			printf("'%s' keychain not found.\n", cname);

			goto exiting;
		}

		/* create the new document */
		db_tmp = xmlNewDoc(BAD_CAST "1.0");
		if (!db_tmp) {
			puts("Could not create the new XML document for export!");

			goto exiting;
		}

		/* A new root node */
		root_node_tmp = xmlNewNode(NULL, BAD_CAST "kc");
		if (!root_node_tmp) {
			puts("Could not create the new root node for export!");

			goto exiting;
		}
		xmlDocSetRootElement(db_tmp, root_node_tmp);

		/* add the specified keychain's node to the export xmlDoc */
		xmlAddChild(root_node_tmp, xmlNewText(BAD_CAST "\n\t"));
		keychain_tmp = xmlCopyNode(keychain, 1);
		if (!keychain_tmp) {
			puts("Could not duplicate keychain for export!");

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
		strlcat(db_params_new.db_filename,
			dump ? ".xml" : ".kcd",
			strlen(db_params_new.db_filename) + 4 + 1);
	}

	if(lstat(db_params_new.db_filename, &st) == 0) {	/* if export filename exists */
		printf("Do you want to overwrite '%s'? <yes/no> ", db_params_new.db_filename);

#ifndef _READLINE
		/* disable history temporarily */
		if (el_set(e, EL_HIST, history, NULL) != 0) {
			perror("el_set(EL_HIST)");
		}
		/* clear the prompt temporarily */
		if (el_set(e, EL_PROMPT, el_prompt_null) != 0) {
			perror("el_set(EL_PROMPT)");
		}

		e_line = el_gets(e, &e_count);

		/* re-enable the default prompt */
		if (el_set(e, EL_PROMPT, prompt_str) != 0) {
			perror("el_set(EL_PROMPT)");
		}
		/* re-enable history */
		if (el_set(e, EL_HIST, history, eh) != 0) {
			perror("el_set(EL_HIST)");
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

		if (strncmp(e_line, "yes", 3) != 0)
			goto exiting;
	}

	if (dump) {
		if (xmlSaveFormatFileEnc(db_params_new.db_filename, db_tmp, "UTF-8", XML_SAVE_FORMAT) > 0) {
			if (chmod(db_params_new.db_filename, S_IRUSR | S_IWUSR) < 0)
				puts("Could not change permissions of dump file!");

			puts("Dump OK");
		} else
			printf("Failed dumping to '%s'.\n", db_params_new.db_filename);
	} else {
		db_params_new.db_file = open(db_params_new.db_filename, O_RDWR | O_CREAT, 0600);
		if (db_params_new.db_file < 0) {
			perror("open(database file)");

			goto exiting;
		}

		bio_chain = kc_setup_bio_chain(db_params_new.db_filename, 1);
		if (!bio_chain) {
			puts("Could not setup bio_chain!");

			close(db_params_new.db_file);
			goto exiting;
		}


		/* ask for the new password */
		while (ret == -1)
			ret = kc_password_read(&db_params_new.pass, 1);

		if (ret == 0) {	/* canceled */
			if (db_params_new.pass)
				memset(db_params_new.pass, '\0', PASSWORD_MAXLEN);
			free(db_params_new.pass); db_params_new.pass = NULL;

			BIO_free_all(bio_chain);
			close(db_params_new.db_file);
			goto exiting;
		}

		/* Generate iv/salt, setup cipher mode and turn on encrypting */
		if (!kc_setup_crypt(bio_chain, 1, &db_params_new, KC_SETUP_CRYPT_IV | KC_SETUP_CRYPT_SALT | KC_SETUP_CRYPT_KEY)) {
			puts("Could not setup encrypting!");

			BIO_free_all(bio_chain);
			close(db_params_new.db_file);

			memset(db_params_new.key, '\0', KEY_LEN);

			if (db_params_new.pass)
				memset(db_params_new.pass, '\0', PASSWORD_MAXLEN);
			free(db_params_new.pass); db_params_new.pass = NULL;

			goto exiting;
		}

		memset(db_params_new.key, '\0', KEY_LEN);

		if (db_params_new.pass)
			memset(db_params_new.pass, '\0', PASSWORD_MAXLEN);
		free(db_params_new.pass); db_params_new.pass = NULL;


		if (kc_db_writer(db_tmp, bio_chain, &db_params_new))
			puts("Export OK");
		else
			printf("Failed exporting to '%s'!\n", db_params_new.db_filename);

		BIO_free_all(bio_chain);
		close(db_params_new.db_file);
	}

exiting:
	if (cname  &&  db_tmp)
		xmlFreeDoc(db_tmp);	/* if we saved a specific keychain, clean up the temporary xmlDoc and its tree. */

	free(cname); cname = NULL;
	free(db_params_new.kdf); db_params_new.kdf = NULL;
	free(db_params_new.cipher); db_params_new.cipher = NULL;
	free(db_params_new.cipher_mode); db_params_new.cipher_mode = NULL;
	free(db_params_new.db_filename); db_params_new.db_filename = NULL;
} /* cmd_export() */
