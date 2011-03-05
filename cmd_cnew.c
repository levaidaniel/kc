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
cmd_cnew(char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*cname_locale = NULL, *cname = NULL;

#ifndef _READLINE
	int		e_count = 0;
#endif


	strtok((char *)e_line, " ");	// remove the command from the line
	cname_locale = BAD_CAST strtok(NULL, " ");	// assign the command's parameter
	if (!cname_locale) {		// if we didn't get a name as a parameter
#ifndef _READLINE
		// disable history temporarily
		if (el_set(e, EL_HIST, eh, NULL) != 0) {
			perror("el_set(EL_HIST)");
		}
#endif

		strlcpy(prompt_context, "NEW keychain", sizeof(prompt_context));


#ifndef _READLINE
		e_line = (char *)el_gets(e, &e_count);

		e_line[strlen(e_line) - 1] = '\0';	// remove the newline
#else
		e_line = readline(prompt_str());
#endif
		if (!e_line) {
			perror("input");
			return;
		} else
			cname_locale = BAD_CAST e_line;

#ifndef _READLINE
		// re-enable history
		if (el_set(e, EL_HIST, eh, history) != 0) {
			perror("el_set(EL_HIST)");
		}
#endif

		strlcpy(prompt_context, "", sizeof(prompt_context));
	}

	cname = convert_utf8(cname_locale, 0);

	db_node = find_keychain(cname);
	if (!db_node) {
		// XXX reloading a saved document inserts a 'text' element between each visible node (why?)
		// so we must reproduce this
		db_node = xmlNewText(BAD_CAST "  ");
		xmlAddChild(keychain->parent, db_node);

		db_node = xmlNewNode(NULL, BAD_CAST "keychain");
		xmlSetProp(db_node, BAD_CAST "name", cname);
		xmlAddChild(keychain->parent, db_node);

		// XXX reloading a saved document inserts a 'text' element between each visible node (why?)
		// so we must reproduce this
		db_node = xmlNewText(BAD_CAST "\n");
		xmlAddChild(keychain->parent, db_node);

		dirty = 1;
	} else {
		cname_locale = convert_utf8(cname, 1);
		printf("'%s' already exists!\n", cname_locale);
		free(cname_locale);
	}

	free(cname);
} /* cmd_cnew() */
