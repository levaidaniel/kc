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
extern char		prompt_context[20];

#ifndef _READLINE
extern EditLine		*e;
extern History		*eh;
extern HistEvent	eh_ev;
#endif


void
cmd_new(const char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*key = NULL, *value = NULL;

	char		*line = NULL;

#ifndef _READLINE
	int		e_count = 0;
#endif


#ifndef _READLINE
	/* disable history temporarily */
	if (el_set(e, EL_HIST, history, NULL) != 0) {
		perror("el_set(EL_HIST)");
	}
#endif

	line = strdup(e_line);

	strtok(line, " ");				/* remove the command from the line */
	key = xmlStrdup(BAD_CAST strtok(NULL, " "));	/* assign the command's first parameter (name) */
	free(line); line = NULL;
	if (!key) {					/* if we didn't get a name as a parameter */
		strlcpy(prompt_context, "NEW key", sizeof(prompt_context));

#ifndef _READLINE
		e_line = el_gets(e, &e_count);
#else
		e_line = readline(prompt_str());
#endif
		if (e_line) {
			key = xmlStrdup(BAD_CAST e_line);
#ifndef _READLINE
			key[xmlStrlen(key) - 1] = '\0';	/* remove the newline */
#endif
		} else {
			perror("input");
#ifndef _READLINE
			el_reset(e);
#endif
			return;
		}
	}
	if (debug)
		printf("new key is '%s'\n", key);


	strlcpy(prompt_context, "NEW value", sizeof(prompt_context));

#ifndef _READLINE
	e_line = el_gets(e, &e_count);
#else
	e_line = readline(prompt_str());
#endif
	if (e_line) {
		value = xmlStrdup(BAD_CAST e_line);
#ifndef _READLINE
		value[xmlStrlen(value) - 1] = '\0';	/* remove the newline */
#endif
	} else {
		perror("input");
#ifndef _READLINE
		el_reset(e);
		xmlFree(key); key = NULL;
#endif
		return;
	}
	if (debug)
		printf("new value is '%s'\n", value);

#ifndef _READLINE
	/* re-enable history */
	if (el_set(e, EL_HIST, history, eh) != 0) {
		perror("el_set(EL_HIST)");
	}
#endif

	strlcpy(prompt_context, "", sizeof(prompt_context));


	/* XXX reloading a saved document inserts a 'text' element between each visible node (why?)
	 * so we must reproduce this */
	db_node = xmlNewText(BAD_CAST "\n    ");
	xmlAddChild(keychain, db_node);

	/* add new element */
	db_node = xmlNewChild(keychain, NULL, BAD_CAST "key", NULL);
	xmlNewProp(db_node, BAD_CAST "name", key);
	xmlNewProp(db_node, BAD_CAST "value", value);
	xmlFree(key); key = NULL;
	xmlFree(value); value = NULL;

	dirty = 1;
} /* cmd_new() */
