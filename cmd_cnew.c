/*
 * Copyright (c) 2011-2013 LEVAI Daniel
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


extern xmlNodePtr	keychain;
extern char		dirty;
extern char		prompt_context[30];

#ifndef _READLINE
extern EditLine		*e;
extern History		*eh;
#endif


void
cmd_cnew(const char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*name = NULL, *description = NULL;

	char		*created = NULL;
	char		*line = NULL;
	int		idx = 0;
#ifndef _READLINE
	int		e_count = 0;
#endif


	line = strdup(e_line);

	strtok(line, " ");				/* remove the command from the line */
	name = xmlStrdup(BAD_CAST strtok(NULL, " "));	/* assign the command's first parameter (name) */
	free(line); line = NULL;
	if (!name) {					/* if we didn't get a name as a parameter */
		strlcpy(prompt_context, "NEW keychain name", sizeof(prompt_context));

#ifndef _READLINE
		/* disable history temporarily */
		if (el_set(e, EL_HIST, history, NULL) != 0) {
			perror("el_set(EL_HIST)");
		}

		e_line = el_gets(e, &e_count);

		/* re-enable history */
		if (el_set(e, EL_HIST, history, eh) != 0) {
			perror("el_set(EL_HIST)");
		}
#else
		e_line = readline(prompt_str());
#endif
		if (e_line) {
			name = xmlStrdup(BAD_CAST e_line);
#ifndef _READLINE
			name[xmlStrlen(name) - 1] = '\0'; /* remove the newline */
#endif
		} else {
#ifndef _READLINE
			el_reset(e);
#endif
			strlcpy(prompt_context, "", sizeof(prompt_context));

			return;
		}
	}

	strlcpy(prompt_context, "NEW keychain description", sizeof(prompt_context));

#ifndef _READLINE
	/* disable history temporarily */
	if (el_set(e, EL_HIST, history, NULL) != 0) {
		perror("el_set(EL_HIST)");
	}

	e_line = el_gets(e, &e_count);

	/* re-enable history */
	if (el_set(e, EL_HIST, history, eh) != 0) {
		perror("el_set(EL_HIST)");
	}
#else
	e_line = readline(prompt_str());
#endif
	if (e_line) {
		description = xmlStrdup(BAD_CAST e_line);
#ifndef _READLINE
		description[xmlStrlen(description) - 1] = '\0'; /* remove the newline */
#endif
	} else {
#ifndef _READLINE
		el_reset(e);
#endif
		strlcpy(prompt_context, "", sizeof(prompt_context));

		return;
	}

	strlcpy(prompt_context, "", sizeof(prompt_context));


	db_node = find_keychain(name, 1);
	if (!db_node) {
		created = malloc(TIME_MAXLEN); malloc_check(created);
		snprintf(created, TIME_MAXLEN, "%d", (int)time(NULL));

		/* XXX reloading a saved document inserts a 'text' element between each visible node (why?)
		 * so we must reproduce this */
		xmlAddChild(keychain->parent, xmlNewText(BAD_CAST "\t"));

		db_node = xmlNewChild(keychain->parent, NULL, BAD_CAST "keychain", NULL);

		xmlNewProp(db_node, BAD_CAST "name", name);
		xmlNewProp(db_node, BAD_CAST "created", BAD_CAST created);
		xmlNewProp(db_node, BAD_CAST "modified", BAD_CAST created);
		xmlNewProp(db_node, BAD_CAST "description", description);

		/* make the XML document prettttyyy */
		xmlAddChild(db_node, xmlNewText(BAD_CAST "\n\t"));

		/* Get the index of the newly added (last) entry */
		db_node = keychain;
		while (db_node) {
			if (db_node->type == XML_ELEMENT_NODE)	/* we only care about ELEMENT nodes */
				idx++;

			db_node = db_node->next;
		}
		printf("Created keychain: %d. %s\n", idx - 1, name);

		/* XXX reloading a saved document inserts a 'text' element between each visible node (why?)
		 * so we must reproduce this */
		xmlAddChild(keychain->parent, xmlNewText(BAD_CAST "\n"));

		dirty = 1;
	} else {
		printf("Keychain '%s' already exists!\n", name);
	}

	xmlFree(name); name = NULL;
	free(created); created = NULL;
} /* cmd_cnew() */
