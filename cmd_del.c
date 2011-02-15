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


extern xmlNodePtr	keychain;


void cmd_del(EditLine *e, ...) {

va_list		ap;

xmlNodePtr	db_node = NULL, db_node_prev = NULL;
xmlChar		*key_locale = NULL, *key = NULL;

char		*line = NULL;
int		idx = 0;

command		*commands = NULL;


	va_start(ap, e);

	line = va_arg(ap, char *);
	line[strlen(line) - 1] = '\0';		/* remove the newline character from the end */

	va_arg(ap, History *);
	va_arg(ap, BIO *);
	commands = va_arg(ap, command *);

	va_end(ap);

	if (sscanf(line, "%*s %d", &idx) <= 0) {
		puts(commands->usage);
		return;
	}
	if (idx < 0) {
		puts(commands->usage);
		return;
	}

	db_node = find_key(idx);
	if (db_node) {
		key = db_node->children->content;
		key_locale = convert_utf8(key, 1);
		printf("'%s'", key_locale);
		if (key_locale)
			free(key_locale);

		db_node_prev = db_node->prev;
		xmlUnlinkNode(db_node_prev);	// remove the adjacent 'text' node, which are the indent and newline
		xmlFreeNode(db_node_prev);

		xmlUnlinkNode(db_node);
		xmlFreeNode(db_node);

		puts(" deleted");
	} else
		puts("invalid entry!");
} /* cmd_del() */
