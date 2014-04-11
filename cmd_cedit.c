/*
 * Copyright (c) 2011-2014 LEVAI Daniel
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
extern xmlChar		*_rl_helper_var;

#ifndef _READLINE
extern EditLine		*e;
extern History		*eh;
#endif


void
cmd_cedit(const char *e_line, command *commands)
{
	xmlChar		*name = NULL, *description = NULL;

	char		*modified = NULL;

#ifndef _READLINE
	int		e_count = 0;
#endif


	strlcpy(prompt_context, "EDIT keychain name", sizeof(prompt_context));

	/* if we edit an existing entry, push the current value to the edit buffer */
	name = xmlGetProp(keychain, BAD_CAST "name");

#ifndef _READLINE
	/* disable history temporarily */
	if (el_set(e, EL_HIST, history, NULL) != 0) {
		perror("el_set(EL_HIST)");
	}

	el_push(e, (const char *)name);
	e_line = el_gets(e, &e_count);

	/* re-enable history */
	if (el_set(e, EL_HIST, history, eh) != 0) {
		perror("el_set(EL_HIST)");
	}
#else
	_rl_helper_var = name;
	rl_pre_input_hook = (rl_hook_func_t *)_rl_push_buffer;
	e_line = readline(prompt_str());
	rl_pre_input_hook = NULL;
#endif
	xmlFree(name); name = NULL;

	if (e_line) {
		name = xmlStrdup(BAD_CAST e_line); malloc_check(name);
#ifndef _READLINE
		name[xmlStrlen(name) - 1] = '\0';	/* remove the newline */
#else
		free((char *)e_line); e_line = NULL;
#endif
	} else {
#ifndef _READLINE
		el_reset(e);
#endif
		strlcpy(prompt_context, "", sizeof(prompt_context));

		return;
	}


	strlcpy(prompt_context, "EDIT keychain description", sizeof(prompt_context));

	/* if we edit an existing entry, push the current value to the edit buffer */
	description = xmlGetProp(keychain, BAD_CAST "description");

#ifndef _READLINE
	/* disable history temporarily */
	if (el_set(e, EL_HIST, history, NULL) != 0) {
		perror("el_set(EL_HIST)");
	}

	if (description)
		el_push(e, (const char *)description);

	e_line = el_gets(e, &e_count);

	/* re-enable history */
	if (el_set(e, EL_HIST, history, eh) != 0) {
		perror("el_set(EL_HIST)");
	}
#else
	if (description) {
		_rl_helper_var = description;
		rl_pre_input_hook = (rl_hook_func_t *)_rl_push_buffer;
	}

	e_line = readline(prompt_str());
	rl_pre_input_hook = NULL;
#endif
	xmlFree(description); description = NULL;

	if (e_line) {
		description = xmlStrdup(BAD_CAST e_line); malloc_check(description);
#ifndef _READLINE
		description[xmlStrlen(description) - 1] = '\0';	/* remove the newline */
#else
		free((char *)e_line); e_line = NULL;
#endif
	} else {
#ifndef _READLINE
		el_reset(e);
#endif
		strlcpy(prompt_context, "", sizeof(prompt_context));

		return;
	}


	xmlSetProp(keychain, BAD_CAST "name", name);
	xmlSetProp(keychain, BAD_CAST "description", description);


	modified = malloc(TIME_MAXLEN); malloc_check(modified);
	snprintf(modified, TIME_MAXLEN, "%d", (int)time(NULL));
	xmlSetProp(keychain, BAD_CAST "modified", BAD_CAST modified);


	xmlFree(name); name = NULL;
	xmlFree(description); description = NULL;
	free(modified); modified = NULL;

	strlcpy(prompt_context, "", sizeof(prompt_context));

	db_params.dirty = 1;
} /* cmd_cedit() */
