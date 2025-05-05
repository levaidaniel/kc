/*
 * Copyright (c) 2011-2024 LEVAI Daniel
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
extern xmlDocPtr	db;
extern xmlNodePtr	keychain;


void
cmd_status(const char *e_line, command *commands)
{
	char	*db_filename_realpath = NULL;
	xmlSaveCtxtPtr		xml_save = NULL;
	xmlBufferPtr		xml_buf = NULL;

#ifdef _HAVE_YUBIKEY
	yk_array	*yk = NULL;
#endif


	/* these are unused in this function */
	e_line = NULL;
	commands = NULL;

	printf("Database file: %s", db_params.db_filename);
	db_filename_realpath = realpath((const char*)db_params.db_filename, NULL);
	if (db_filename_realpath)
		printf(" (%s)", db_filename_realpath);
	printf("\n");
	free(db_filename_realpath); db_filename_realpath = NULL;

	xml_buf = xmlBufferCreate();
	xml_save = xmlSaveToBuffer(xml_buf, "UTF-8", XML_SAVE_FORMAT);
	xmlSaveDoc(xml_save, db);
	xmlSaveFlush(xml_save);
	printf("XML structure size: %d bytes\n", (int)xmlBufferLength(xml_buf));
	xmlSaveClose(xml_save);
	xmlBufferFree(xml_buf);

	printf("Security key(s): ");
#ifdef _HAVE_YUBIKEY
	yk = db_params.yk;

	if (yk) {
		puts("");

		while (yk) {
			printf(" YubiKey slot #%d on device #%d, serial: %d\n", yk->slot, yk->dev, yk->serial);
			yk = yk->next;
		}
	} else {
		puts("no");
	}
#else
	puts("no");
#endif

	printf("Password: ");
	if (	(strlen(db_params.ssha_type)  &&  db_params.ssha_password)  ||
		db_params.yk_password  ||
		(!strlen(db_params.ssha_type)  &&  !db_params.yk)
	)
		puts("yes");
	else
		puts("no");

	printf("SSH agent: ");
	if (strlen(db_params.ssha_type))
		printf("(%s) %s\n", db_params.ssha_type, db_params.ssha_comment);
	else
		puts("no");

	printf("Password function: %s (", db_params.kdf);
#ifdef _HAVE_LIBSCRYPT
	if (strcmp(db_params.kdf, "scrypt") == 0)
		printf("N(%lu), r(%lu), p(%lu)", db_params.first ? strtoul(db_params.first, NULL, 10) : SCRYPT_N, db_params.second ? strtoul(db_params.second, NULL, 10) : SCRYPT_r, db_params.third ? strtoul(db_params.third, NULL, 10) : SCRYPT_p);
	else
#endif
#ifdef _HAVE_ARGON2
	if (strcmp(db_params.kdf, "argon2id") == 0)
		printf("%lu iterations, %lu memory lanes, %luk memory cost", db_params.kdf_reps, db_params.first ? strtoul(db_params.first, NULL, 10) : KC_ARGON2ID_LANES, db_params.second ? strtoul(db_params.second, NULL, 10) : KC_ARGON2ID_MEMCOST);
	else
#endif
	printf("%lu %s", db_params.kdf_reps, strncmp(db_params.kdf, "bcrypt", 6) == 0 ? "rounds" : "iterations");
	printf(")\n");

	printf("Key length: %lu bytes / %lu bits\n", db_params.key_len, db_params.key_len * 8);

	printf("Encryption: %s", db_params.cipher);
	if (strncmp(db_params.cipher_mode, "n/a", 4) == 0) {
		printf("\n");
	} else {
		printf(", %s\n", db_params.cipher_mode);
	}

	printf("Read-only: %s\n", (db_params.readonly ? "yes" : "no"));

	if (!db_params.readonly)
		printf("Modified: %s\n", (db_params.dirty ? "yes" : "no"));
} /* cmd_status() */
