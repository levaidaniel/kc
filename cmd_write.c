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


#include "common.h"
#include "commands.h"


extern xmlDocPtr	db;

extern BIO		*bio_chain;

extern int		db_file;

extern char		dirty;

extern unsigned char	salt[17], iv[17];


void
cmd_write(const char *e_line, command *commands)
{
	xmlSaveCtxtPtr		xml_save = NULL;
	xmlBufferPtr		xml_buf = NULL;

	int			ret = 0, remaining = 0;


	xml_buf = xmlBufferCreate();
	xml_save = xmlSaveToBuffer(xml_buf, "UTF-8", XML_SAVE_FORMAT);

	if (xml_save) {
		xmlSaveDoc(xml_save, db);
		xmlSaveFlush(xml_save);
		if (getenv("KC_DEBUG"))
			printf("xml_buf content:\n'%s'(%d)\n", xmlBufferContent(xml_buf), (int)xmlBufferLength(xml_buf));
		xmlSaveClose(xml_save);


		/* rewrite the database */
		if (ftruncate(db_file, 0) != 0) {
			puts("There was an error while trying to save the XML document!");
			if (getenv("KC_DEBUG"))
				perror("db file truncate");

			return;
		}
		lseek(db_file, 0, SEEK_SET);

		/* write the IV and salt first */
		write(db_file, iv, sizeof(iv) - 1);
		write(db_file, salt, sizeof(salt) - 1);


		BIO_reset(bio_chain);		/* we must reset the cipher BIO to work after subsequent calls to cmd_write() */

		BIO_seek(bio_chain, sizeof(iv) - 1 + sizeof(salt) - 1);	/* seek after the IV and salt */

		remaining = xmlBufferLength(xml_buf);
		while (remaining > 0) {
			ret = BIO_write(bio_chain, xmlBufferContent(xml_buf), remaining);

			if (ret <= 0) {
				if (BIO_should_retry(bio_chain)) {
					if (getenv("KC_DEBUG"))
						puts("write delay");

					sleep(1);
					continue;
				} else {
					if (getenv("KC_DEBUG"))
						puts("BIO_write() error (don't retry)");

					puts("There was an error while trying to save the XML document!");

					break;
				}
			}

			remaining -= ret;

			if (getenv("KC_DEBUG")) {
				printf("wrote: %d\n", ret);
				printf("remaining: %d\n", remaining);
			}
		}

		do {
			if (BIO_flush(bio_chain) == 1) {
				if (getenv("KC_DEBUG"))
					puts("flushed bio_chain");
			} else {
				if (BIO_should_retry(bio_chain)) {
					if (getenv("KC_DEBUG"))
						puts("flush delay");

					sleep(1);
					continue;
				} else {
					if (getenv("KC_DEBUG"))
						puts("BIO_should_retry() is false");

					puts("There was an error while trying to save the XML document!");

					break;
				}
			}
		} while(BIO_wpending(bio_chain) > 0);

		if (getenv("KC_DEBUG"))
			printf("db_file size -> %d\n", BIO_tell(bio_chain));

		xmlBufferFree(xml_buf);

		puts("Save OK");

		dirty = 0;
	} else {
		if (getenv("KC_DEBUG"))
			puts("xmlSaveToBuffer() error");

		puts("There was an error while trying to save the XML document!");
	}
} /* cmd_write() */
