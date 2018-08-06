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


extern db_parameters	db_params;
extern xmlDocPtr	db;
extern xmlNodePtr	keychain;


void
cmd_status(const char *e_line, command *commands)
{
	char	*db_filename_realpath = NULL;
	xmlSaveCtxtPtr		xml_save = NULL;
	xmlBufferPtr		xml_buf = NULL;


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

	printf("SSH agent: ");
	if (strlen(db_params.ssha_type))
		printf("(%s) %s\n", db_params.ssha_type, db_params.ssha_comment);
	else
		puts("No");

	printf("Password handling: %s\n", db_params.kdf);

	printf("Encryption: %s, %s\n", db_params.cipher, db_params.cipher_mode);

	printf("Read-only: %s\n", (db_params.readonly ? "yes" : "no"));

	if (!db_params.readonly)
		printf("Modified: %s\n", (db_params.dirty ? "yes" : "no"));
} /* cmd_status() */
