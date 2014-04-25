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

#ifdef BSD
#include <fcntl.h>
#endif

#if defined(__linux__)  ||  defined(__CYGWIN__)
#include <sys/file.h>
#endif


extern db_parameters	db_params;
extern xmlDocPtr	db;


void
cmd_write(const char *e_line, command *commands)
{
	db_parameters	db_params_tmp;
	BIO		*bio_chain_tmp = NULL;

	struct stat	st;
	char		*rand_str = NULL;


	/* initial db_params for the exported database */
	db_params_tmp.pass = NULL;
	db_params_tmp.db_filename = malloc(MAXPATHLEN);
	if (!db_params_tmp.db_filename) {
		perror("Could not allocate memory for the file name");
		return;
	}
	db_params_tmp.db_file = -1;
	db_params_tmp.pass_filename = NULL;
	db_params_tmp.dirty = 0;
	db_params_tmp.readonly = 0;
	db_params_tmp.kdf = db_params.kdf;
	db_params_tmp.cipher = db_params.cipher;
	db_params_tmp.cipher_mode = db_params.cipher_mode;
	if (strlcpy(	(char *)db_params_tmp.iv,
			(const char *)db_params.iv,
			IV_DIGEST_LEN + 1)
			>= IV_DIGEST_LEN + 1)
	{
		puts("Could not duplicate the original IV!");

		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		return;
	}
	if (strncmp((const char *)db_params_tmp.iv, (const char *)db_params.iv, IV_DIGEST_LEN) != 0) {
		puts("The new and the original IV do not match!");

		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		return;
	}

	if (strlcpy(	(char *)db_params_tmp.salt,
			(const char *)db_params.salt,
			SALT_DIGEST_LEN + 1)
			>= SALT_DIGEST_LEN + 1)
	{
		puts("Could not duplicate the original salt!");

		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		return;
	}
	if (strncmp((const char *)db_params_tmp.salt, (const char *)db_params.salt, SALT_DIGEST_LEN) != 0) {
		puts("The new and the original salt do not match!");

		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		return;
	}

	/* the key gets duplicated right before the call to kc_setup_crypt() */


	if (strlcpy(db_params_tmp.db_filename, db_params.db_filename, MAXPATHLEN) >= MAXPATHLEN) {
		puts("Could not construct a temporary filename!");

		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		return;
	}

	rand_str = get_random_str(6, 0);
	if (!rand_str) {
		puts("Could not create a random string for a temporary filename!");

		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		return;
	}
	if (strlcat(db_params_tmp.db_filename, rand_str, MAXPATHLEN) >= MAXPATHLEN) {
		puts("Could not construct a temporary filename #2!");

		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		free(rand_str); rand_str = NULL;
		return;
	}

	free(rand_str); rand_str = NULL;

	if (stat(db_params_tmp.db_filename, &st) == 0) {	/* if temporary database filename exists */
		puts("Could not create temporary database file (exists)!");

		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		return;
	}

	db_params_tmp.db_file = open(db_params_tmp.db_filename, O_RDWR | O_CREAT, 0600);
	if (db_params_tmp.db_file < 0) {
		puts("Could not open temporary database file!");
		perror("open(tmp db_filename)");

		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		return;
	}

	/* setup temporary bio_chain */
	bio_chain_tmp = kc_setup_bio_chain(db_params_tmp.db_filename, 1);
	if (!bio_chain_tmp) {
		printf("Could not setup bio_chain_tmp!");

		close(db_params_tmp.db_file);
		unlink(db_params_tmp.db_filename);
		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		return;
	}


	memcpy(db_params_tmp.key, db_params.key, KEY_LEN);
	if (memcmp(db_params_tmp.key, db_params.key, KEY_LEN) != 0) {
		puts("Could not duplicate the original key!");

		memset(db_params_tmp.key, '\0', KEY_LEN);
		BIO_free_all(bio_chain_tmp);
		close(db_params_tmp.db_file);
		unlink(db_params_tmp.db_filename);
		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		return;
	}
	/* Setup cipher mode, turn on encrypting, etc... */
	if (!kc_setup_crypt(bio_chain_tmp, 1, &db_params_tmp, 0)) {
		printf("Could not setup encrypting!");

		memset(db_params_tmp.key, '\0', KEY_LEN);
		BIO_free_all(bio_chain_tmp);
		close(db_params_tmp.db_file);
		unlink(db_params_tmp.db_filename);
		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		return;
	}
	memset(db_params_tmp.key, '\0', KEY_LEN);


	if (!kc_db_writer(db, bio_chain_tmp, &db_params_tmp)) {
		puts("There was an error while trying to save the database!");

		BIO_free_all(bio_chain_tmp);
		close(db_params_tmp.db_file);
		unlink(db_params_tmp.db_filename);
		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		return;
	}


	stat(db_params_tmp.db_filename, &st);
	if (getenv("KC_DEBUG"))
		printf("temporary database file has been written: %d bytes\n", (int)st.st_size);

	if (st.st_size <= IV_DIGEST_LEN + SALT_DIGEST_LEN + 2) {
		puts("Temporary database file became unusually small!");

		BIO_free_all(bio_chain_tmp);
		close(db_params_tmp.db_file);
		unlink(db_params_tmp.db_filename);
		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		return;
	}


	if (close(db_params.db_file) == 0) {
		if (getenv("KC_DEBUG"))
			puts("closed old database file");
	} else
		perror("close(old database file)");


	if (close(db_params_tmp.db_file) == 0) {
		if (getenv("KC_DEBUG"))
			puts("closed tmp database file");
	} else
		perror("close(tmp database file)");


	if (rename(db_params_tmp.db_filename, db_params.db_filename) < 0) {
		puts("Could not rename temporary database file!");
		perror("rename(tmp db_filename, db_filename)");

		BIO_free_all(bio_chain_tmp);
		close(db_params_tmp.db_file);
		unlink(db_params_tmp.db_filename);
		free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;
		return;
	} else
		if (getenv("KC_DEBUG"))
			puts("renamed temporary database file to the original database filename");

	BIO_free_all(bio_chain_tmp);
	unlink(db_params_tmp.db_filename);
	free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;


	/* Reopen the new database file as the 'db_file' fd. */
	db_params.db_file = open(db_params.db_filename, O_RDONLY);
	if (db_params.db_file < 0) {
		puts("Could not reopen the new database file! This means that a file lock can not be placed on it. I suggest you to restart the application!");
		perror("open(new database file)");

		return;
	} else
		if (getenv("KC_DEBUG"))
			puts("reopened new database file");

	if (flock(db_params.db_file, LOCK_NB | LOCK_EX) < 0) {
		if (getenv("KC_DEBUG"))
			puts("flock(new database file)");

		puts("Could not lock the new database file! I suggest you to restart the application!");
		perror("flock(new database file)");

		return;
	} else
		if (getenv("KC_DEBUG"))
			puts("locked new database file");


	db_params.dirty = 0;

	puts("Save OK");
} /* cmd_write() */
