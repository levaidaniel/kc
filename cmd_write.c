/*
 * Copyright (c) 2011, 2012, 2013 LEVAI Daniel
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
 * DISCLAIMED. IN NO EVENT SHALL LEVAI Daniel BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <sys/stat.h>
#ifndef _LINUX
#include <sys/syslimits.h>
#include <fcntl.h>
#else
#include <linux/limits.h>
#include <sys/file.h>
#endif

#include "common.h"
#include "commands.h"


extern xmlDocPtr	db;

extern BIO		*bio_chain;
extern char		*cipher_mode;
extern unsigned char	salt[SALT_LEN + 1], iv[IV_LEN + 1], key[KEY_LEN];

extern int		db_file;
extern char		*db_filename;

extern char		dirty;


void
cmd_write(const char *e_line, command *commands)
{
	BIO		*bio_chain_tmp = NULL;

	struct stat	st;
	char		*rand_str = NULL;
	char		db_filename_tmp[PATH_MAX];
	int		db_file_tmp = 0;


	rand_str = get_random_str(6, 1);
	if (!rand_str)
		return;

	strlcpy(db_filename_tmp, db_filename, PATH_MAX);
	strlcat(db_filename_tmp, rand_str, PATH_MAX);
	free(rand_str); rand_str = NULL;
	if(stat(db_filename_tmp, &st) == 0) {	/* if temporary database filename exists */
		puts("Couldn't create temporary database file (exists)!");

		return;
	}

	db_file_tmp = open(db_filename_tmp, O_RDWR | O_CREAT, 0600);
	if (db_file_tmp < 0) {
		puts("Couldn't open temporary database file!");
		perror("open(db_file_tmp)");

		return;
	}

	/* setup temporary bio_chain */
	bio_chain_tmp = kc_setup_bio_chain(db_filename_tmp);
	if (!bio_chain_tmp) {
		printf("Couldn't setup bio_chain_tmp!");

		close(db_file_tmp);
		unlink(db_filename_tmp);
		return;
	}

	/* Setup cipher mode and turn on encrypting */
	if (!kc_setup_crypt(bio_chain_tmp, 1, cipher_mode, NULL, iv, NULL, key, 0)) {
		printf("Couldn't setup encrypting!");

		BIO_free_all(bio_chain_tmp);
		close(db_file_tmp);
		unlink(db_filename_tmp);
		return;
	}

	if (!kc_db_writer(db_file_tmp, db, bio_chain_tmp, iv, salt)) {
		puts("There was an error while trying to save the database!");

		BIO_free_all(bio_chain_tmp);
		close(db_file_tmp);
		unlink(db_filename_tmp);
		return;
	}

	if (rename(db_filename_tmp, db_filename) < 0) {
		puts("Couldn't rename temporary database file!");
		perror("open(db_file_tmp)");

		BIO_free_all(bio_chain_tmp);
		close(db_file_tmp);
		unlink(db_filename_tmp);
		return;
	}

	puts("Save OK");

	dirty = 0;

	BIO_free_all(bio_chain_tmp);
	close(db_file_tmp);
	unlink(db_filename_tmp);
} /* cmd_write() */
