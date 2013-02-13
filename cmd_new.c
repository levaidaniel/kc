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


extern xmlNodePtr	keychain;
extern char		dirty;
extern char		prompt_context[20];

#ifndef _READLINE
extern EditLine		*e;
extern History		*eh;
#endif


void
cmd_new(const char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*key = NULL, *value_rR = NULL, *value = NULL;

	char		*created = NULL;
	char		*line = NULL;
	int		idx = 0;

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
#ifndef _READLINE
			el_reset(e);

			/* re-enable history */
			if (el_set(e, EL_HIST, history, eh) != 0) {
				perror("el_set(EL_HIST)");
			}
#endif
			strlcpy(prompt_context, "", sizeof(prompt_context));

			return;
		}
	}
	if (getenv("KC_DEBUG"))
		printf("new key is '%s'\n", key);


	strlcpy(prompt_context, "NEW value", sizeof(prompt_context));

#ifndef _READLINE
	e_line = el_gets(e, &e_count);

	/* re-enable history */
	if (el_set(e, EL_HIST, history, eh) != 0) {
		perror("el_set(EL_HIST)");
	}
#else
	e_line = readline(prompt_str());
#endif
	if (e_line) {
		value_rR = xmlStrdup(BAD_CAST e_line);
#ifndef _READLINE
		value_rR[xmlStrlen(value_rR) - 1] = '\0';	/* remove the newline */
#endif
	} else {
#ifndef _READLINE
		el_reset(e);
#endif
		strlcpy(prompt_context, "", sizeof(prompt_context));

		xmlFree(key); key = NULL;
		return;
	}
	value = parse_randoms(value_rR);

	if (getenv("KC_DEBUG"))
		printf("new value is '%s'\n", value);

	strlcpy(prompt_context, "", sizeof(prompt_context));


	created = malloc(TIME_MAXLEN); malloc_check(created);
	snprintf(created, TIME_MAXLEN, "%d", (int)time(NULL));

	/* XXX reloading a saved document inserts a 'text' element between each visible node (why?)
	 * so we must reproduce this */
	xmlAddChild(keychain, xmlNewText(BAD_CAST "\t"));

	/* add new element */
	db_node = xmlNewChild(keychain, NULL, BAD_CAST "key", NULL);

	xmlNewProp(db_node, BAD_CAST "name", key);
	xmlNewProp(db_node, BAD_CAST "value", value);
	xmlNewProp(db_node, BAD_CAST "created", BAD_CAST created);
	xmlNewProp(db_node, BAD_CAST "modified", BAD_CAST created);

	/* Get the index of the newly added (last) entry */
	db_node = keychain->children;
	while (db_node) {
		if (db_node->type == XML_ELEMENT_NODE)	/* we only care about ELEMENT nodes */
			idx++;

		db_node = db_node->next;
	}
	printf("Created: %d. %s\n", idx - 1, key);

	xmlFree(key); key = NULL;
	xmlFree(value_rR); value_rR = NULL;
	xmlFree(value); value = NULL;
	free(created); created = NULL;

	/* make the XML document prettttyyy */
	xmlAddChild(keychain, xmlNewText(BAD_CAST "\n\t"));

	dirty = 1;
} /* cmd_new() */
