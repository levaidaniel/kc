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


#include <sys/stat.h>
#ifndef _LINUX
#include <fcntl.h>
#else
#include <sys/file.h>
#endif

#include "common.h"
#include "commands.h"


#ifndef _READLINE
extern EditLine		*e;
extern History		*eh;
#endif

extern xmlDocPtr	db;

extern char		*cipher_mode;


void
cmd_export(const char *e_line, command *commands)
{
	BIO		*bio_chain = NULL;

	xmlDocPtr	db_tmp = NULL;
	xmlNodePtr	keychain = NULL, keychain_tmp = NULL, root_node_tmp = NULL;
	xmlChar		*cname = NULL;

	char		*export_filename = NULL, *line = NULL;
	int		export_file = 0;
	char		*cmd = NULL, dump = 0, ret = -1;
	char		*pass = NULL;
	unsigned char	iv[IV_LEN + 1], salt[SALT_LEN + 1], key[KEY_LEN];
	struct stat	st;

#ifndef _READLINE
	int		e_count = 0;
#endif


	line = strdup(e_line);

	cmd = strtok(line, " ");		/* get the command name */
	if (strcmp(cmd, "dump") == 0)
		dump = 1;

	export_filename = strtok(NULL, " ");	/* assign the command's parameter */
	if (!export_filename) {
		puts(commands->usage);
		free(line); line = NULL;
		return;
	}
	export_filename = strdup(export_filename);

	cname = BAD_CAST strtok(NULL, " ");	/* assign the command's parameter */
	if (cname) {
		/* A 'keychain' was specified, so export only that one.
		 * We must create a new xmlDoc and copy the specified keychain to it.
		 */

		keychain = find_keychain(cname, 0);
		if (!keychain) {
			printf("'%s' keychain not found.\n", cname);

			free(export_filename); export_filename = NULL;
			free(line); line = NULL;
			return;
		}

		/* create the new document */
		db_tmp = xmlNewDoc(BAD_CAST "1.0");
		if (!db_tmp) {
			puts("Could not create the new XML document for export!");

			free(export_filename); export_filename = NULL;
			free(line); line = NULL;
			return;
		}

		/* A new root node */
		root_node_tmp = xmlNewNode(NULL, BAD_CAST "kc");
		if (!root_node_tmp) {
			puts("Could not create the new root node for export!");

			xmlFreeDoc(db_tmp);
			free(export_filename); export_filename = NULL;
			free(line); line = NULL;
			return;
		}
		xmlDocSetRootElement(db_tmp, root_node_tmp);

		/* add the specified keychain's node to the export xmlDoc */
		xmlAddChild(root_node_tmp, xmlNewText(BAD_CAST "\n\t"));
		keychain_tmp = xmlCopyNode(keychain, 1);
		if (!keychain_tmp) {
			puts("Could not duplicate keychain for export!");

			xmlFreeDoc(db_tmp);
			free(export_filename); export_filename = NULL;
			free(line); line = NULL;
			return;
		}
		xmlAddChild(root_node_tmp, keychain_tmp);

		xmlAddChild(root_node_tmp, xmlNewText(BAD_CAST "\n"));
	} else
		/* save the whole document */
		db_tmp = db;

	/* Check if the supplied filename contains an extension */
	if (!strchr(export_filename, '.')) {
		export_filename = realloc(export_filename, strlen(export_filename) + 4); malloc_check(export_filename);
		strlcat(export_filename, dump ? ".xml" : ".kcd", strlen(export_filename) + 4 + 1);
	}

	if(lstat(export_filename, &st) == 0) {	/* if export filename exists */
#ifndef _READLINE
		/* disable history temporarily */
		if (el_set(e, EL_HIST, history, NULL) != 0) {
			perror("el_set(EL_HIST)");
		}
		/* clear the prompt temporarily */
		if (el_set(e, EL_PROMPT, el_prompt_null) != 0) {
			perror("el_set(EL_PROMPT)");
		}
#endif
		printf("Do you want to overwrite '%s'? <yes/no> ", export_filename);

#ifdef _READLINE
		rl_redisplay();
#endif

#ifndef _READLINE
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
		e_line = readline("");
#endif
		if (!e_line) {
#ifndef _READLINE
			el_reset(e);
#endif
			free(export_filename); export_filename = NULL;
			free(line); line = NULL;
			return;
		}

		if (strncmp(e_line, "yes", 3) != 0) {
			free(export_filename); export_filename = NULL;
			free(line); line = NULL;
			return;
		}
	}

	if (dump) {
		if (xmlSaveFormatFileEnc(export_filename, db_tmp, "UTF-8", XML_SAVE_FORMAT) > 0) {
			if (chmod(export_filename, S_IRUSR | S_IWUSR) < 0)
				puts("Could not change permissions of dump file!");

			puts("Dump OK");
		} else
			printf("Failed dumping to '%s'.\n", export_filename);
	} else {
		/* ask for the new password */
		while (ret == -1)
			ret = kc_password_read(&pass, 1);

		if (ret == 0) {	/* canceled */
			BIO_free_all(bio_chain);
			free(export_filename); export_filename = NULL;
			free(line); line = NULL;
			return;
		}


		export_file = open(export_filename, O_RDWR | O_CREAT, 0600);
		if (export_file < 0) {
			perror("open(database file)");

			free(export_filename); export_filename = NULL;
			free(line); line = NULL;

			if (pass)
				memset(pass, '\0', PASSWORD_MAXLEN);
			free(pass); pass = NULL;

			return;
		}

		bio_chain = kc_setup_bio_chain(export_filename);
		if (!bio_chain) {
			printf("Could not setup bio_chain!");

			close(export_file);
			free(export_filename); export_filename = NULL;
			free(line); line = NULL;

			if (pass)
				memset(pass, '\0', PASSWORD_MAXLEN);
			free(pass); pass = NULL;

			return;
		}


		/* Generate iv/salt, setup cipher mode and turn on encrypting */
		if (!kc_setup_crypt(bio_chain, 1, cipher_mode, pass, iv, salt, key, KC_SETUP_CRYPT_IV | KC_SETUP_CRYPT_SALT | KC_SETUP_CRYPT_KEY)) {
			printf("Could not setup encrypting!");

			BIO_free_all(bio_chain);
			close(export_file);
			free(export_filename); export_filename = NULL;
			free(line); line = NULL;

			if (pass)
				memset(pass, '\0', PASSWORD_MAXLEN);
			free(pass); pass = NULL;

			return;
		}

		if (pass)
			memset(pass, '\0', PASSWORD_MAXLEN);
		free(pass); pass = NULL;


		if (kc_db_writer(export_file, db_tmp, bio_chain, iv, salt))
			puts("Export OK");
		else
			printf("Failed exporting to '%s'!\n", export_filename);

		BIO_free_all(bio_chain);
		close(export_file);
	}

	if (cname)
		xmlFreeDoc(db_tmp);	/* if we saved a specific keychain, clean up the temporary xmlDoc and its tree. */

	free(export_filename); export_filename = NULL;
	free(line); line = NULL;
} /* cmd_export() */
