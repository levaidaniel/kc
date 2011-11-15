/*
 * Copyright (c) 2011 LEVAI Daniel
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


#include <stdarg.h>

#include "common.h"
#include "commands.h"


extern xmlDocPtr	db;
extern char		dirty;
extern BIO		*bio_chain;


void
cmd_write(const char *e_line, command *commands)
{
	xmlSaveCtxtPtr		xml_save = NULL;
	xmlBufferPtr		xml_buf = NULL;

	int			ret = 0;


	xml_buf = xmlBufferCreate();
	xml_save = xmlSaveToBuffer(xml_buf, "UTF-8", XML_SAVE_FORMAT);

	if (xml_save) {
		xmlSaveDoc(xml_save, db);
		xmlSaveFlush(xml_save);
		if (debug)
			printf("xml_buf->content:\n'%s'\n", xmlBufferContent(xml_buf));
		xmlSaveClose(xml_save);

		BIO_reset(bio_chain);		/* we must reset the cipher BIO to work after subsequent calls to cmd_write() */
		BIO_seek(bio_chain, 32);	/* seek after the IV and salt (both 16 bytes) */

		ret = BIO_write(bio_chain, xml_buf->content, (int)xml_buf->use);
		if (debug)
			printf("wrote %d bytes\n", ret);
		switch (ret) {
			case -2:
				if (debug)
					puts("unsupported operation!");
			break;
		}

		do {
			BIO_flush(bio_chain);
			if (debug)
				puts("flushed bio_chain");
		} while(BIO_wpending(bio_chain));

		xmlBufferFree(xml_buf);

		dirty = 0;
	} else {
		puts("XML save error");
	}
} /* cmd_write() */
