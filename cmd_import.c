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


extern db_parameters	db_params;
extern xmlDocPtr	db;
extern xmlNodePtr	keychain;


void
cmd_import(const char *e_line, command *commands)
{
	BIO		*bio_chain = NULL;

	struct stat	st;

	db_parameters	db_params_new;

	xmlDocPtr	db_new = NULL;
	xmlNodePtr	db_root = NULL, db_root_new = NULL, entry_new = NULL,
			keychain_new = NULL, keychain_cur = NULL;
	xmlChar		*cname = NULL;

	char		*cmd = NULL, *kdf = NULL, *cipher_mode = NULL, append = 0, xml = 0;
	char		*line = NULL;
	char		*buf = NULL;
	ssize_t		ret = -1;
	int		pos = 0;


	/* initial db_params parameters of the imported database */
	db_params_new.pass = NULL;
	db_params_new.db_filename = NULL;
	db_params_new.db_file = -1;
	db_params_new.pass_filename = NULL;
	db_params_new.dirty = 0;
	db_params_new.readonly = 0;


	line = strdup(e_line);

	cmd = strtok(line, " ");		/* get the command name */
	if (strncmp(cmd, "append", 6) == 0)	/* command is 'append' or 'appendxml' */
		append = 1;

	if (strcmp(cmd + 6, "xml") == 0)
		xml = 1;

	db_params_new.db_filename = strtok(NULL, " ");	/* assign the command's parameter */
	if (!db_params_new.db_filename) {
		puts(commands->usage);

		free(line); line = NULL;
		return;
	}

	kdf = strtok(NULL, " ");		/* assign the command's second parameter (kdf) */
	/* Changed KDF? */
	if (kdf)
		db_params_new.kdf = kdf;
	else
		db_params_new.kdf = db_params.kdf;

	cipher_mode = strtok(NULL, " ");	/* assign the command's third parameter (cipher_mode) */
	/* Changed cipher mode? */
	if (cipher_mode)
		db_params_new.cipher_mode = cipher_mode;
	else
		db_params_new.cipher_mode = db_params.cipher_mode;


	if (xml) {
		/* plain text XML database import */

		if (getenv("KC_DEBUG"))
			db_new = xmlReadFile(db_params_new.db_filename, "UTF-8", XML_PARSE_NONET | XML_PARSE_PEDANTIC);
		else
			db_new = xmlReadFile(db_params_new.db_filename, "UTF-8", XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);

		if (!db_new) {
			xmlGenericError(xmlGenericErrorContext, "Failed to parse XML from '%s'.\n", db_params_new.db_filename);

			free(line); line = NULL;
			return;
		}
	} else {
		/* encrypted database import */

		/* This should be identical with what is in kc.c */
		if(stat(db_params_new.db_filename, &st) == 0) {

			printf("Opening '%s'\n",db_params_new.db_filename);

			if(!S_ISLNK(st.st_mode)  &&  !S_ISREG(st.st_mode)) {
				printf("'%s' is not a regular file or a link!\n", db_params_new.db_filename);

				free(line); line = NULL;
				return;
			}

			if(st.st_size == 0) {
				printf("'%s' is an empty file!\n", db_params_new.db_filename);

				free(line); line = NULL;
				return;
			}

			if(st.st_size <= IV_DIGEST_LEN + SALT_DIGEST_LEN + 2) {
				printf("'%s' is suspiciously small file!\n", db_params_new.db_filename);

				free(line); line = NULL;
				return;
			}

			db_params_new.db_file = open(db_params_new.db_filename, O_RDONLY);
			if (db_params_new.db_file < 0) {
				perror("open(database file)");

				free(line); line = NULL;
				return;
			}

			/* read the IV */
			buf = malloc(IV_DIGEST_LEN + 1); malloc_check(buf);
			ret = 0; pos = 0;
			do {
				ret = read(db_params_new.db_file, buf + pos, IV_DIGEST_LEN - pos);
				pos += ret;
			} while (ret > 0  &&  pos < IV_DIGEST_LEN);
			buf[pos] = '\0';

			if (ret < 0) {
				perror("read IV(database file)");

				close(db_params_new.db_file);
				free(buf); buf = NULL;
				free(line); line = NULL;
				return;
			}
			if (pos != IV_DIGEST_LEN) {
				puts("Could not read IV from database file!");

				close(db_params_new.db_file);
				free(buf); buf = NULL;
				free(line); line = NULL;
				return;
			} else
				strlcpy((char *)db_params_new.iv, (const char *)buf, IV_DIGEST_LEN + 1);

			free(buf); buf = NULL;

			if (getenv("KC_DEBUG"))
				printf("iv='%s'\n", db_params_new.iv);

			/* Fast-forward one byte after the current position,
			 * to skip the newline.
			 */
			lseek(db_params_new.db_file, 1, SEEK_CUR);

			/* read the salt */
			buf = malloc(SALT_DIGEST_LEN + 1); malloc_check(buf);
			ret = 0; pos = 0;
			do {
				ret = read(db_params_new.db_file, buf + pos, SALT_DIGEST_LEN - pos);
				pos += ret;
			} while (ret > 0  &&  pos < SALT_DIGEST_LEN);
			buf[pos] = '\0';

			if (ret < 0) {
				perror("read salt(database file)");

				close(db_params_new.db_file);
				free(buf); buf = NULL;
				free(line); line = NULL;
				return;
			}
			if (pos != SALT_DIGEST_LEN) {
				puts("Could not read salt from database file!");

				close(db_params_new.db_file);
				free(buf); buf = NULL;
				free(line); line = NULL;
				return;
			} else
				strlcpy((char *)db_params_new.salt, (const char *)buf, SALT_DIGEST_LEN + 1);

			free(buf); buf = NULL;

			if (getenv("KC_DEBUG"))
				printf("salt='%s'\n", db_params_new.salt);


			if (close(db_params_new.db_file) < 0) {
				perror("close after reading IV/salt(database file)");

				free(line); line = NULL;
				return;
			}
		} else {
			perror("Could not open database file");

			free(line); line = NULL;
			return;
		}

		bio_chain = kc_setup_bio_chain(db_params_new.db_filename, 0);
		if (!bio_chain) {
			printf("Could not setup bio_chain!");

			free(line); line = NULL;
			return;
		}

		/* ask for the password */
		kc_password_read(&db_params_new.pass, 0);

		/* Setup cipher mode and turn on decrypting */
		ret = kc_setup_crypt(bio_chain, 0, &db_params_new, KC_SETUP_CRYPT_KEY);

		/* from here on now, we don't need to store the key or the password text anymore */
		memset(db_params_new.key, '\0', KEY_LEN);

		if (db_params_new.pass)
			memset(db_params_new.pass, '\0', PASSWORD_MAXLEN);
		free(db_params_new.pass); db_params_new.pass = NULL;

		/* kc_setup_crypt() check from a few lines above */
		if (!ret) {
			printf("Could not setup decrypting!");

			BIO_free_all(bio_chain);
			free(line); line = NULL;
			return;
		}


		ret = kc_db_reader(&buf, bio_chain);
		if (getenv("KC_DEBUG"))
			printf("read %d bytes\n", (int)ret);

		if (BIO_get_cipher_status(bio_chain) == 0  &&  ret > 0) {
			puts("Failed to decrypt import file!");

			BIO_free_all(bio_chain);
			free(buf); buf = NULL;
			free(line); line = NULL;
			return;
		}

		BIO_free_all(bio_chain);

		if (getenv("KC_DEBUG"))
			db_new = xmlReadMemory(buf, (int)ret, NULL, "UTF-8", XML_PARSE_NONET | XML_PARSE_RECOVER);
		else
			db_new = xmlReadMemory(buf, (int)ret, NULL, "UTF-8", XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_RECOVER);

		free(buf); buf = NULL;

		if (!db_new) {
			puts("Could not parse XML document!");

			free(line); line = NULL;
			return;
		}
	}


	if (!kc_validate_xml(db_new)) {
		printf("Not a valid kc XML structure ('%s')!\n", db_params_new.db_filename);

		xmlFreeDoc(db_new);
		free(line); line = NULL;
		return;
	}


	if (append) {
		db_root = xmlDocGetRootElement(db);	/* the existing db root */
		db_root_new = xmlDocGetRootElement(db_new);

		if (db_root_new->children->next == NULL) {
			puts("Won't append from an empty database!");

			xmlFreeDoc(db_new);
			free(line); line = NULL;
			return;
		}

		/* extract the keychain from the document being appended */
		keychain_new = db_root_new->children->next;

		/* We would like to append every keychain that is in the source file,
		 * hence the loop. */
		while (keychain_new) {
			if (keychain_new->type == XML_ELEMENT_NODE) {	/* we only care about ELEMENT nodes */
				cname = xmlGetProp(keychain_new, BAD_CAST "name");
				keychain_cur = find_keychain(cname, 1);	/* search for an existing keychain name in the current db */
				xmlFree(cname); cname = NULL;

				/* If an existing keychain name is encountered,
				 * append the entries from the imported keychain to
				 * the existing keychain, and don't add a duplicate
				 * keychain.
				 */
				if (keychain_cur) {
					entry_new = keychain_new->children->next;
					while (entry_new) {
						if (entry_new->type == XML_ELEMENT_NODE) {	/* we only care about ELEMENT nodes */
							xmlAddChild(keychain_cur, xmlNewText(BAD_CAST "\t"));
							xmlAddChild(keychain_cur, xmlCopyNode(entry_new, 1));
							xmlAddChild(keychain_cur, xmlNewText(BAD_CAST "\n\t"));
						}

						entry_new = entry_new->next;
					}
				} else {
					/* create a non-existing keychain */

					xmlAddChild(db_root, xmlNewText(BAD_CAST "\t"));
					xmlAddChild(db_root, xmlCopyNode(keychain_new, 1));
					xmlAddChild(db_root, xmlNewText(BAD_CAST "\n"));
				}
			}

			keychain_new = keychain_new->next;
		}

		xmlFreeDoc(db_new);
	} else {
		db_root_new = xmlDocGetRootElement(db_new);
		if (db_root_new->children->next == NULL) {
			puts("Won't import an empty database!");

			xmlFreeDoc(db_new);
			free(line); line = NULL;
			return;
		}

		keychain = db_root_new->children->next;

		xmlFreeDoc(db);
		db = db_new;
	}

	db_params.dirty = 1;

	if (append)
		puts("Append OK");
	else
		puts("Import OK");

	free(line); line = NULL;
} /* cmd_import() */
