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
extern xmlChar		*_rl_helper_var;

#ifndef _READLINE
extern EditLine		*e;
extern History		*eh;
extern HistEvent	eh_ev;
#endif


void
cmd_cren(char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*cname_locale = NULL, *cname = NULL;

#ifndef _READLINE
	int		e_count = 0;
#endif


	strtok((char *)e_line, " ");	/* remove the command from the line */
	cname_locale = BAD_CAST strtok(NULL, " ");	/* assign the command's parameter */
	if (!cname_locale) {
		puts(commands->usage);
		return;
	}

	cname = convert_utf8(cname_locale, 0);

	db_node = find_keychain(cname);
	if (db_node) {
#ifndef _READLINE
		// disable history temporarily
		if (el_set(e, EL_HIST, eh, NULL) != 0) {
			perror("el_set(EL_HIST)");
		}
#endif

		strlcpy(prompt_context, "RENAME keychain", sizeof(prompt_context));

		// if we edit an existing entry, push the current value to the edit buffer
		free(cname);
		cname = xmlGetProp(db_node, BAD_CAST "name");
		cname_locale = convert_utf8(cname, 1);
#ifdef _READLINE
		_rl_helper_var = cname_locale;
#endif

#ifndef _READLINE
		el_push(e, (const char *)cname_locale);

		e_line = (char *)el_gets(e, &e_count);

		if (e_line)
			e_line[strlen(e_line) - 1] = '\0';	// remove the newline
#else
		rl_pre_input_hook = (rl_hook_func_t *)_rl_push_buffer;
		e_line = readline(prompt_str());
		rl_pre_input_hook = NULL;
#endif
		if (!e_line) {
			perror("input");
			return;
		} else {
			free(cname_locale); cname_locale = NULL;
			cname_locale = BAD_CAST e_line;
		}

		cname = convert_utf8(cname_locale, 0);


		xmlSetProp(db_node, BAD_CAST "name", cname);

		free(cname);

#ifndef _READLINE
		// re-enable history
		if (el_set(e, EL_HIST, eh, history) != 0) {
			perror("el_set(EL_HIST)");
		}
#endif

		strlcpy(prompt_context, "", sizeof(prompt_context));

		dirty = 1;
	} else {
		printf("keychain '%s' not found.\n", cname_locale);
		free(cname);
	}
} /* cmd_cren() */
