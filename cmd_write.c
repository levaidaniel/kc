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


	/* these are unused in this function */
	e_line = NULL;
	commands = NULL;

	/* initial db_params for the exported database */
	db_params_tmp.pass = NULL;
	db_params_tmp.db_file = -1;
	db_params_tmp.db_filename = malloc(MAXPATHLEN);
	if (!db_params_tmp.db_filename) {
		perror("Could not allocate memory for the file name");
		goto exiting;
	}
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
		dprintf(STDERR_FILENO, "ERROR: Could not duplicate the original IV!\n");

		goto exiting;
	}
	if (strncmp((const char *)db_params_tmp.iv, (const char *)db_params.iv, IV_DIGEST_LEN) != 0) {
		puts("The new and the original IV do not match!");

		goto exiting;
	}

	if (strlcpy(	(char *)db_params_tmp.salt,
			(const char *)db_params.salt,
			SALT_DIGEST_LEN + 1)
			>= SALT_DIGEST_LEN + 1)
	{
		dprintf(STDERR_FILENO, "ERROR: Could not duplicate the original salt!\n");

		goto exiting;
	}
	if (strncmp((const char *)db_params_tmp.salt, (const char *)db_params.salt, SALT_DIGEST_LEN) != 0) {
		puts("The new and the original salt do not match!");

		goto exiting;
	}

	/* the key gets duplicated right before the call to kc_crypt_setup() */


	if (strlcpy(db_params_tmp.db_filename, db_params.db_filename, MAXPATHLEN) >= MAXPATHLEN) {
		dprintf(STDERR_FILENO, "ERROR: Could not construct a temporary filename!\n");

		goto exiting;
	}

	rand_str = get_random_str(6, 0);
	if (!rand_str) {
		dprintf(STDERR_FILENO, "ERROR: Could not create a random string for a temporary filename!\n");

		goto exiting;
	}
	if (strlcat(db_params_tmp.db_filename, rand_str, MAXPATHLEN) >= MAXPATHLEN) {
		dprintf(STDERR_FILENO, "ERROR: Could not construct a temporary filename #2!\n");

		goto exiting;
	}

	if (stat(db_params_tmp.db_filename, &st) == 0) {	/* if temporary database filename exists */
		perror("Could not create temporary database file (exists)!");

		goto exiting;
	}

	db_params_tmp.db_file = open(db_params_tmp.db_filename, O_RDWR | O_CREAT, 0600);
	if (db_params_tmp.db_file < 0) {
		perror("Could not open temporary database file");

		goto exiting;
	}

	/* setup temporary bio_chain */
	bio_chain_tmp = kc_setup_bio_chain(db_params_tmp.db_filename, 1);
	if (!bio_chain_tmp) {
		dprintf(STDERR_FILENO, "ERROR: Could not setup bio_chain_tmp!\n");

		goto exiting;
	}


	memcpy(db_params_tmp.key, db_params.key, KEY_LEN);
	if (memcmp(db_params_tmp.key, db_params.key, KEY_LEN) != 0) {
		dprintf(STDERR_FILENO, "ERROR: Could not duplicate the original key!\n");

		goto exiting;
	}
	/* Setup cipher mode, turn on encrypting, etc... */
	if (!kc_crypt_setup(bio_chain_tmp, 1, &db_params_tmp)) {
		dprintf(STDERR_FILENO, "ERROR: Could not setup encrypting!\n");

		goto exiting;
	}
	memset(db_params_tmp.key, '\0', KEY_LEN);


	if (!kc_db_writer(db, bio_chain_tmp, &db_params_tmp)) {
		dprintf(STDERR_FILENO, "ERROR: There was an error while trying to save the database!\n");

		goto exiting;
	}


	stat(db_params_tmp.db_filename, &st);
	if (getenv("KC_DEBUG"))
		printf("%s(): temporary database file has been written: %d bytes\n", __func__, (int)st.st_size);

	if (st.st_size <= IV_DIGEST_LEN + SALT_DIGEST_LEN + 2) {
		puts("Temporary database file became unusually small!");

		goto exiting;
	}


	if (close(db_params.db_file) == 0) {
		if (getenv("KC_DEBUG"))
			printf("%s(): closed old database file\n", __func__);
	} else
		perror("close(old database file)");


	if (close(db_params_tmp.db_file) == 0) {
		if (getenv("KC_DEBUG"))
			printf("%s(): closed tmp database file\n", __func__);
	} else
		perror("close(tmp database file)");


	if (rename(db_params_tmp.db_filename, db_params.db_filename) < 0) {
		dprintf(STDERR_FILENO, "ERROR: Could not rename temporary database file!\n");
		perror("rename(tmp db_filename, db_filename)");

		goto exiting;
	} else
		if (getenv("KC_DEBUG"))
			printf("%s(): renamed temporary database file to the original database filename\n", __func__);


	/* Reopen the new database file as the 'db_file' fd. */
	db_params.db_file = open(db_params.db_filename, O_RDONLY);
	if (db_params.db_file < 0) {
		dprintf(STDERR_FILENO, "ERROR: Could not reopen the new database file! This means that a file lock can not be placed on it. I suggest you to restart the application!\n");
		perror("open(new database file)");

		goto exiting;
	} else
		if (getenv("KC_DEBUG"))
			printf("%s(): reopened new database file\n", __func__);

	if (flock(db_params.db_file, LOCK_NB | LOCK_EX) < 0) {
		if (getenv("KC_DEBUG"))
			printf("%s(): flock(new database file)\n", __func__);

		dprintf(STDERR_FILENO, "ERROR: Could not lock the new database file! I suggest you to restart the application!\n");
		perror("flock(new database file)");

		goto exiting;
	} else
		if (getenv("KC_DEBUG"))
			printf("%s(): locked new database file\n", __func__);


	db_params.dirty = 0;

	puts("Save OK");

exiting:
	free(rand_str); rand_str = NULL;

	if (db_params_tmp.db_file >= 0)
		close(db_params_tmp.db_file);
	if (db_params_tmp.db_filename)
		unlink(db_params_tmp.db_filename);
	free(db_params_tmp.db_filename); db_params_tmp.db_filename = NULL;

	if (bio_chain_tmp)
		BIO_free_all(bio_chain_tmp);

	memset(db_params_tmp.key, '\0', KEY_LEN);
} /* cmd_write() */
