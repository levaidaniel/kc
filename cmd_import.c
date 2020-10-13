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

#ifdef BSD
#include <fcntl.h>
#endif

#if defined(__linux__)  ||  defined(__CYGWIN__)
#include <sys/file.h>
#endif


extern db_parameters	db_params;
extern xmlDocPtr	db;
extern xmlNodePtr	keychain;


void
cmd_import(const char *e_line, command *commands)
{
	BIO		*bio_chain = NULL;

	struct stat	st;

	extra_parameters	params;

	db_parameters	db_params_new;

	xmlDocPtr	db_new = NULL;
	xmlNodePtr	db_root = NULL, db_root_new = NULL, entry_new = NULL,
			keychain_new = NULL, keychain_cur = NULL, db_node = NULL;
	xmlChar		*cname = NULL;

	xmlChar		*description = NULL;
	xmlChar		*created = NULL;
	xmlChar		*modified = NULL;
	char		*created_now = NULL;

	int		c = 0, largc = 0;
	size_t		len = 0;
	char		*opts = NULL;
	char		**largv = NULL;
	char		*line = NULL;
	char		*buf = NULL;
	char		append = 0, xml = 0;
	ssize_t		ret = -1;
	int		pos = 0;

	unsigned long int	count_keychains = 0, count_keys = 0, count_keys_new = 0;


	params.caller = "import";
	params.legacy = 0;

	/* initial db_params parameters of the imported database */
	db_params_new.ssha_type[0] = '\0';
	db_params_new.ssha_comment[0] = '\0';
	db_params_new.ssha_password = 0;
	db_params_new.yk = 0;
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
	opts = "A:k:P:R:e:m:Y:o";
#else
	opts = "A:k:P:R:e:m:o";
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

	/* print some status information after parsing the options */
	if (	(strlen(db_params_new.ssha_type)  &&  db_params_new.yk)  &&
		(!db_params_new.ssha_password  ||  !db_params_new.yk_password)
	) {
		dprintf(STDERR_FILENO, "ERROR: Using -A and -Y together only makes sense with the ',password' parameter for both of them!\n");
		goto exiting;
	}

	if (strlen(db_params_new.ssha_type))
		printf("Using (%s) %s identity%s\n", db_params_new.ssha_type, db_params_new.ssha_comment, (db_params_new.ssha_password ? " and a password" : ""));


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
			quit(EXIT_FAILURE);
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


	if (strncmp(largv[0], "append", 6) == 0)	/* command is 'append' or 'appendxml' */
		append = 1;

	if (strcmp(largv[0] + 6, "xml") == 0)
		xml = 1;


	if (!db_params_new.db_filename) {
		puts(commands->usage);
		goto exiting;
	}


	puts("Reading database...");


	if (xml) {
		/* plain text XML database import */

		if (getenv("KC_DEBUG"))
			db_new = xmlReadFile(db_params_new.db_filename, "UTF-8", XML_PARSE_NONET | XML_PARSE_PEDANTIC | XML_PARSE_RECOVER);
		else
			db_new = xmlReadFile(db_params_new.db_filename, "UTF-8", XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);

		if (!db_new) {
			xmlGenericError(xmlGenericErrorContext, "ERROR: Failed to parse XML from '%s'", db_params_new.db_filename);
			if (errno == 0)
				puts("");
			else
				dprintf(STDERR_FILENO, "ERROR: : %s\n", strerror(errno));

			goto exiting;
		}
	} else {
		/* encrypted database import */

		/* This should be identical to what is in kc.c */
		if (stat(db_params_new.db_filename, &st) == 0) {

			printf("Opening '%s'\n",db_params_new.db_filename);

			if (!S_ISLNK(st.st_mode)  &&  !S_ISREG(st.st_mode)) {
				dprintf(STDERR_FILENO, "ERROR: '%s' is not a regular file or a link!\n", db_params_new.db_filename);
				goto exiting;
			}

			if (st.st_size == 0) {
				dprintf(STDERR_FILENO, "ERROR: '%s' is an empty file!\n", db_params_new.db_filename);
				goto exiting;
			}

			if (st.st_size <= IV_DIGEST_LEN + SALT_DIGEST_LEN + 2) {
				dprintf(STDERR_FILENO, "ERROR: '%s' is a suspiciously small file!\n", db_params_new.db_filename);
				goto exiting;
			}

			db_params_new.db_file = open(db_params_new.db_filename, O_RDONLY);
			if (db_params_new.db_file < 0) {
				perror("ERROR: open(database file)");
				goto exiting;
			}

			/* read the IV */
			pos = 0;
			do {
				ret = read(db_params_new.db_file, db_params_new.iv + pos, IV_DIGEST_LEN - pos);
				pos += ret;
			} while (ret > 0  &&  pos < IV_DIGEST_LEN);
			db_params_new.iv[pos] = '\0';

			if (ret < 0) {
				perror("ERROR: import: read IV(database file)");
				goto exiting;
			}
			if (pos != IV_DIGEST_LEN) {
				dprintf(STDERR_FILENO, "ERROR: Could not read IV from database file!\n");
				goto exiting;
			}

			if (getenv("KC_DEBUG"))
				printf("%s(): import: iv='%s'\n", __func__, db_params_new.iv);

			/* Fast-forward one byte after the current position,
			 * to skip the newline.
			 */
			lseek(db_params_new.db_file, 1, SEEK_CUR);

			/* read the salt */
			pos = 0;
			do {
				ret = read(db_params_new.db_file, db_params_new.salt + pos, SALT_DIGEST_LEN - pos);
				pos += ret;
			} while (ret > 0  &&  pos < SALT_DIGEST_LEN);
			db_params_new.salt[pos] = '\0';

			if (ret < 0) {
				perror("ERROR: import: read salt(database file)");
				goto exiting;
			}
			if (pos != SALT_DIGEST_LEN) {
				dprintf(STDERR_FILENO, "ERROR: Could not read salt from database file!\n");
				goto exiting;
			}

			if (getenv("KC_DEBUG"))
				printf("%s(): import: salt='%s'\n", __func__, db_params_new.salt);
		} else {
			perror("ERROR: Could not open database file");
			goto exiting;
		}

		bio_chain = kc_setup_bio_chain(db_params_new.db_filename, 0);
		if (!bio_chain) {
			dprintf(STDERR_FILENO, "ERROR: Could not setup bio_chain!\n");
			goto exiting;
		}

		/* Get a password into the database */
		if (kc_crypt_pass(&db_params_new, 0) != 1) {
				dprintf(STDERR_FILENO, "ERROR: Could not get a password!\n");
				goto exiting;
		}

		puts("Decrypting...");

		/* Setup cipher mode and turn on decrypting */
		ret = kc_crypt_key(&db_params_new)  &&  kc_crypt_setup(bio_chain, 0, &db_params_new);

		/* from here on now, we don't need to store the key or the password text anymore */
		memset(db_params_new.key, '\0', KEY_LEN);
		if (db_params_new.pass)
			memset(db_params_new.pass, '\0', db_params_new.pass_len);
		free(db_params_new.pass); db_params_new.pass = NULL;

		/* kc_crypt_key() and kc_crypt_setup() check from a few lines above */
		if (!ret) {
			dprintf(STDERR_FILENO, "ERROR: Could not setup decrypting!\n");
			goto exiting;
		}


		ret = kc_db_reader(&buf, bio_chain);
		if (getenv("KC_DEBUG"))
			printf("%s(): read %d bytes\n", __func__, (int)ret);

		if (BIO_get_cipher_status(bio_chain) == 0  &&  ret > 0) {
			dprintf(STDERR_FILENO, "ERROR: Failed to decrypt import file!\n");

			free(buf); buf = NULL;
			goto exiting;
		}

		BIO_free_all(bio_chain); bio_chain = NULL;

		if (getenv("KC_DEBUG"))
			db_new = xmlReadMemory(buf, (int)ret, NULL, "UTF-8", XML_PARSE_NONET | XML_PARSE_PEDANTIC | XML_PARSE_RECOVER);
		else
			db_new = xmlReadMemory(buf, (int)ret, NULL, "UTF-8", XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);

		free(buf); buf = NULL;

		if (!db_new) {
			dprintf(STDERR_FILENO, "ERROR: Could not parse XML document!\n");
			goto exiting;
		}
	}


	puts("Checking database...");


	if (!kc_validate_xml(db_new, params.legacy)) {
		dprintf(STDERR_FILENO, "ERROR: Not a valid kc XML structure ('%s')!\n", db_params_new.db_filename);

		xmlFreeDoc(db_new);
		goto exiting;
	}


	db_root = xmlDocGetRootElement(db);		/* the existing db root */
	db_root_new = xmlDocGetRootElement(db_new);	/* the new db root */
	if (db_root_new->children->next == NULL) {
		dprintf(STDERR_FILENO, "ERROR: Cannot import an empty database!\n");

		xmlFreeDoc(db_new);
		goto exiting;
	}


	puts("Counting keys and keychains...");

	if (append) {
		/* Extract the keychain from the document being appended */
		keychain_new = db_root_new->children->next;

		/* Count the keychains in the current database */
		count_keychains = count_elements(keychain->parent->children);
		if (count_keychains >= ITEMS_MAX - 1) {
			dprintf(STDERR_FILENO, "ERROR: Maximum number of keychains (%lu) already reached in current database.\n", ITEMS_MAX - 1);

			xmlFreeDoc(db_new);
			goto exiting;
		}

		if (count_elements(keychain_new->parent->children) >= ITEMS_MAX) {
			dprintf(STDERR_FILENO, "ERROR: Number of keychains in the database being imported is larger than the allowed maximum, %lu.\n", ITEMS_MAX - 1);

			xmlFreeDoc(db_new);
			goto exiting;
		}

		/* We would like to append every keychain that is in the source database,
		 * hence the loop.
		 */
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
					/* Range check
					 * See, if the new keys would fit into the existing keychain.
					 */
					if (count_elements(keychain_cur->children->next) >= ITEMS_MAX - 1) {
						dprintf(STDERR_FILENO, "ERROR: Maximum number of keys (%lu) in current keychain already reached.\n", ITEMS_MAX - 1);

						xmlFreeDoc(db_new);
						goto exiting;
					}

					count_keys_new = count_elements(keychain_new->children->next);
					while (count_keys_new > 0  &&  count_keys < ITEMS_MAX) {
						count_keys++;
						count_keys_new--;
					}

					if (count_keys_new == 0  &&  count_keys < ITEMS_MAX) {
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
						dprintf(STDERR_FILENO, "ERROR: Keys from keychain '%s' would not fit in the existing keychain; could not append.\n", xmlGetProp(keychain_new, BAD_CAST "name"));

						xmlFreeDoc(db_new);
						goto exiting;
					}
				} else {
					if (count_elements(keychain_new->parent->children) >= ITEMS_MAX) {
						dprintf(STDERR_FILENO, "ERROR: Number of keys in the keychain being imported (%s) is larger than the allowed maximum, %lu.\n", xmlGetProp(keychain_new, BAD_CAST "name"), ITEMS_MAX - 1);

						xmlFreeDoc(db_new);
						goto exiting;
					}

					/* Range check
					 * See, if the new keychain would fit into the current database.
					 */
					if (count_keychains >= ITEMS_MAX - 1) {
						dprintf(STDERR_FILENO, "ERROR: Cannot copy keychain from database being imported: maximum number of keychains reached, %lu.\n", ITEMS_MAX - 1);

						xmlFreeDoc(db_new);
						goto exiting;
					} else {
						/* Create a non-existing keychain */
						xmlAddChild(db_root, xmlNewText(BAD_CAST "\t"));
						xmlAddChild(db_root, xmlCopyNode(keychain_new, 1));
						xmlAddChild(db_root, xmlNewText(BAD_CAST "\n"));

						count_keychains++;
					}
				}
			}

			keychain_new = keychain_new->next;
		}

		xmlFreeDoc(db_new);

		puts("Append finished.");
	} else {
		/* Range and attribute checks */
		keychain_new = db_root_new->children->next;

		created_now = malloc(TIME_MAXLEN);
		if (!created_now) {
			dprintf(STDERR_FILENO, "ERROR: Could not allocate memory for 'created' attribute!\n");

			xmlFreeDoc(db_new);
			goto exiting;
		}
		snprintf(created_now, TIME_MAXLEN, "%d", (int)time(NULL));


		/* Iterate through keychains and count the keys in them */
		while (keychain_new  &&  count_keychains < ITEMS_MAX) {
			if (keychain_new->type != XML_ELEMENT_NODE) {	/* we only care about ELEMENT nodes */
				keychain_new = keychain_new->next;
				continue;
			}

			count_keychains++;


			/* Fill in missing but mandatory keychain attributes */
			description = xmlGetProp(keychain_new, BAD_CAST "description");
			if (description) {
				xmlFree(description); description = NULL;
			} else {
				xmlNewProp(keychain_new, BAD_CAST "description", BAD_CAST "");
			}

			modified = xmlGetProp(keychain_new, BAD_CAST "modified");
			if (!modified)
				xmlNewProp(keychain_new, BAD_CAST "modified", BAD_CAST created_now);

			created = xmlGetProp(keychain_new, BAD_CAST "created");
			if (created) {
				xmlFree(created); created = NULL;
			} else {
				/* If there's no 'created' attribute but
				 * there's 'modified', use that for the
				 * creation date.
				 * This can be the case eg. after modifying
				 * an older database with newer kc, which would
				 * update the 'modified' attribute but wouldn't
				 * create a 'created' one.
				 */
				if (modified)
					xmlNewProp(keychain_new, BAD_CAST "created", modified);
				else
					xmlNewProp(keychain_new, BAD_CAST "created", BAD_CAST created_now);
			}
			xmlFree(modified); modified = NULL;


			/* Count the keys in the keychain */
			db_node = keychain_new->children;
			keychain_new = keychain_new->next;

			/* Fill in missing but mandatory key attributes */
			while (db_node) {
				if (db_node->type == XML_ELEMENT_NODE) {	/* we only care about ELEMENT nodes */
					count_keys++;
					if (count_keys >= ITEMS_MAX) {
						dprintf(STDERR_FILENO, "ERROR: Number of keys in the keychain being imported (%s) is larger than the allowed maximum, %lu.\n", xmlGetProp(keychain_new, BAD_CAST "name"), ITEMS_MAX - 1);

						xmlFreeDoc(db_new);
						goto exiting;
					}


					modified = xmlGetProp(db_node, BAD_CAST "modified");
					if (!modified)
						xmlNewProp(db_node, BAD_CAST "modified", BAD_CAST created_now);

					created = xmlGetProp(db_node, BAD_CAST "created");
					if (created) {
						xmlFree(created); created = NULL;
					} else {
						/* If there's no 'created' attribute but
						 * there's 'modified', use that for the
						 * creation date.
						 * This can be the case eg. after modifying
						 * an older database with newer kc, which would
						 * update the 'modified' attribute but wouldn't
						 * create a 'created' one.
						 */
						if (modified)
							xmlNewProp(db_node, BAD_CAST "created", modified);
						else
							xmlNewProp(db_node, BAD_CAST "created", BAD_CAST created_now);
					}
					xmlFree(modified); modified = NULL;
				}

				db_node = db_node->next;
			}
		}

		/* Finished scanning the keys in the new keychains, now lets evaluate the number of keychains */
		if (count_keychains >= ITEMS_MAX) {
			dprintf(STDERR_FILENO, "ERROR: Number of keychains in the database being imported is larger than the allowed maximum, %lu.\n", ITEMS_MAX - 1);

			xmlFreeDoc(db_new);
			goto exiting;
		}


		keychain = db_root_new->children->next;

		xmlFreeDoc(db);
		db = db_new;

		puts("Import finished.");
	}

	db_params.dirty = 1;

exiting:
	for (c = 0; c < largc; c++) {
		free(largv[c]); largv[c] = NULL;
	}
	free(largv); largv = NULL;

	if (bio_chain) {
		BIO_free_all(bio_chain);
		bio_chain = NULL;
	}

	xmlFree(description); description = NULL;
	xmlFree(created); created = NULL;
	free(created_now); created_now = NULL;
	xmlFree(modified); modified = NULL;

	if (db_params_new.db_file >= 0)
		close(db_params_new.db_file);

	memset(db_params_new.key, '\0', KEY_LEN);
	if (db_params_new.pass)
		memset(db_params_new.pass, '\0', db_params_new.pass_len);
	free(db_params_new.pass); db_params_new.pass = NULL;

	free(db_params_new.kdf); db_params_new.kdf = NULL;
	free(db_params_new.cipher); db_params_new.cipher = NULL;
	free(db_params_new.cipher_mode); db_params_new.cipher_mode = NULL;
	free(db_params_new.db_filename); db_params_new.db_filename = NULL;
} /* cmd_import() */
