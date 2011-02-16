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
extern char		dirty;


void
cmd_edit(EditLine *e, ...)
{
	va_list		ap;

	History		*eh = NULL;

	xmlNodePtr	db_node = NULL, db_node_new = NULL;
	xmlChar		*key_locale = NULL, *key = NULL, *value_locale = NULL, *value_nl = NULL, *value = NULL;

	command		*commands = NULL;

	char		*line = NULL;
	const char	*e_line = NULL;
	int		idx = 0, e_count = 0;


	va_start(ap, e);

	line = va_arg(ap, char *);
	line[strlen(line) - 1] = '\0';		/* remove the newline character from the end */

	eh = va_arg(ap, History *);
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
		// disable history temporarily
		if (el_set(e, EL_HIST, history, NULL) != 0) {
			perror("el_set(EL_HIST)");
		}


		// set the new edit prompt
		if (el_set(e, EL_CLIENTDATA, "EDIT key") != 0) {
			perror("el_set(EL_CLIENTDATA)");
		}
		// if we edit an existing entry, push the current value to the edit buffer
		key = xmlNodeGetContent(db_node->children);
		key_locale = convert_utf8(key, 1);
		if (key) {
			xmlFree(key); key = NULL;
		}
		el_push(e, (const char *)key_locale);
		free(key_locale); key_locale = NULL;


		e_line = el_gets(e, &e_count);
		if (!e_line) {
			perror("input");
			return;
		} else
			key_locale = BAD_CAST e_line;

		key_locale[xmlStrlen(key_locale) - 1] = '\0';		// remove the newline character from the end
		key = convert_utf8(key_locale, 0);


		// set the new edit prompt
		if (el_set(e, EL_CLIENTDATA, "EDIT value") != 0) {
			perror("el_set(EL_CLIENTDATA)");
		}
		// if we edit an existing entry, push the current value to the edit buffer
		value = xmlNodeGetContent(db_node->children->next->children);
		value_nl = parse_newlines(value, 1);
		if (value) {
			xmlFree(value); value = NULL;
		}
		value_locale = convert_utf8(value_nl, 1);
		free(value_nl); value_nl = NULL;


		el_push(e, (const char *)value_locale);
		free(value_locale); value_locale = NULL;


		e_line = el_gets(e, &e_count);
		if (!e_line) {
			perror("input");
			return;
		} else
			value_locale = BAD_CAST e_line;

		value_locale[xmlStrlen(value_locale) - 1] = '\0';	// remove the newline character from the end
		value_locale = parse_newlines(value_locale, 0);
		value = convert_utf8(value_locale, 0);
		free(value_locale);


		db_node_new = xmlNewNode(NULL, BAD_CAST "key");
		xmlAddChild(db_node_new, xmlNewText(key));
		xmlNewTextChild(db_node_new, NULL, BAD_CAST "value", value);

		db_node = xmlReplaceNode(db_node, db_node_new);
		xmlFreeNode(db_node);


		free(key);
		free(value);

		// change back to the default prompt
		if (el_set(e, EL_CLIENTDATA, "") != 0) {
			perror("el_set(EL_CLIENTDATA)");
		}
		// re-enable history
		if (el_set(e, EL_HIST, history, eh) != 0) {
			perror("el_set(EL_HIST)");
		}

		dirty = 1;
	} else
		puts("invalid entry!");
} /* cmd_edit() */
