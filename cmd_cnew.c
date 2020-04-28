/*
 * Copyright (c) 2011-2020 LEVAI Daniel
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
extern xmlNodePtr	keychain;
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

	char			*created = NULL;
	char			*line = NULL;
	unsigned long int	idx = 0;
	int			i = 0;
#ifndef _READLINE
	int		e_count = 0;
#endif


	/* this is unused in this function */
	commands = NULL;

	if ((idx = count_elements(keychain->parent->children)) >= ITEMS_MAX - 1) {
		dprintf(STDERR_FILENO, "ERROR: Can not create the keychain: maximum number of keychains reached, %lu.\n", ITEMS_MAX - 1);

		return;
	}


#ifndef _READLINE
	/* disable history temporarily */
	if (el_set(e, EL_HIST, history, NULL) != 0) {
		perror("ERROR: el_set(EL_HIST)");
	}
#endif

	line = strdup(e_line); malloc_check(line);

	/* Search for the first space in the command line, to see if
	 * a key name was specified. If it was, the space will be right
	 * after the command's name.
	 */
	for (i = 0; line[i] != ' '  &&  line[i] != '\0'; i++) {}

	/* Search for the first non-space character after the first space
	 * after the command name; this will be start of the key's name
	 * (if it's been specified).
	 * If no keyname was specified, then it will be a zero sized string,
	 * and we check for that.
	 */
	for (; line[i] == ' '; i++) {}

	name = xmlStrdup(BAD_CAST &line[i]); malloc_check(name);
	free(line); line = NULL;

	if (xmlStrlen(name) <= 0) {	/* if we didn't get a keychain name as a parameter */
		strlcpy(prompt_context, "NEW keychain name", sizeof(prompt_context));

#ifndef _READLINE
		e_line = el_gets(e, &e_count);
#else
		e_line = readline(prompt_str());
#endif
		if (e_line) {
			name = xmlStrdup(BAD_CAST e_line); malloc_check(name);
#ifndef _READLINE
			name[xmlStrlen(name) - 1] = '\0'; /* remove the newline */
#else
			free((char *)e_line); e_line = NULL;
#endif
		} else {
#ifndef _READLINE
			el_reset(e);
#endif
			strlcpy(prompt_context, "", sizeof(prompt_context));

			xmlFree(name); name = NULL;
			return;
		}
	}
	if (getenv("KC_DEBUG"))
		printf("%s(): new keychain is '%s'\n", __func__, name);


	strlcpy(prompt_context, "NEW keychain description", sizeof(prompt_context));

#ifndef _READLINE
	e_line = el_gets(e, &e_count);

	/* re-enable history */
	if (el_set(e, EL_HIST, history, eh) != 0) {
		perror("ERROR: el_set(EL_HIST)");
	}
#else
	e_line = readline(prompt_str());
#endif
	if (e_line) {
		description = xmlStrdup(BAD_CAST e_line); malloc_check(description);
#ifndef _READLINE
		description[xmlStrlen(description) - 1] = '\0'; /* remove the newline */
#else
		free((char *)e_line); e_line = NULL;
#endif
	} else {
#ifndef _READLINE
		el_reset(e);
#endif
		strlcpy(prompt_context, "", sizeof(prompt_context));

		xmlFree(name); name = NULL;
		return;
	}

	if (getenv("KC_DEBUG"))
		printf("%s(): new keychain description is '%s'\n", __func__, description);

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

		printf("Created keychain: %lu. %s\n", idx, name);

		/* XXX reloading a saved document inserts a 'text' element between each visible node (why?)
		 * so we must reproduce this */
		xmlAddChild(keychain->parent, xmlNewText(BAD_CAST "\n"));

		db_params.dirty = 1;
	} else {
		printf("Keychain '%s' already exists!\n", name);
	}

	xmlFree(name); name = NULL;
	xmlFree(description); description = NULL;
	free(created); created = NULL;
} /* cmd_cnew() */
