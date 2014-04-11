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
cmd_edit(const char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*key = NULL, *value_rR = NULL, *value = NULL;

	char		*modified = NULL;
	char		*line = NULL, *cmd = NULL, *inv = NULL;

#ifndef _READLINE
	int		e_count = 0;
#endif
	unsigned long int	idx = 0;


	line = strdup(e_line); malloc_check(line);


	cmd = strtok(line, " ");
	if (!cmd) {
		puts(commands->usage);

		free(line); line = NULL;
		return;
	}


	cmd = strtok(NULL, " ");	/* first parameter, the index number */
	if (!cmd) {
		puts(commands->usage);

		free(line); line = NULL;
		return;
	}

	errno = 0;
	idx = strtoul((const char *)cmd, &inv, 10);
	if (inv[0] != '\0'  ||  errno != 0  ||  cmd[0] == '-') {
		puts(commands->usage);

		free(line); line = NULL;
		return;
	}


	free(line); line = NULL;


	db_node = find_key(idx);
	if (db_node) {
#ifndef _READLINE
		/* disable history temporarily */
		if (el_set(e, EL_HIST, history, NULL) != 0) {
			perror("el_set(EL_HIST)");
		}
#endif

		strlcpy(prompt_context, "EDIT key", sizeof(prompt_context));

		/* if we edit an existing entry, push the current value to the edit buffer */
		key = xmlGetProp(db_node, BAD_CAST "name");

#ifndef _READLINE
		el_push(e, (const char *)key);
		e_line = el_gets(e, &e_count);
#else
		_rl_helper_var = key;
		rl_pre_input_hook = (rl_hook_func_t *)_rl_push_buffer;
		e_line = readline(prompt_str());
		rl_pre_input_hook = NULL;
#endif
		xmlFree(key); key = NULL;

		if (e_line) {
			key = xmlStrdup(BAD_CAST e_line); malloc_check(key);
#ifndef _READLINE
			key[xmlStrlen(key) - 1] = '\0';	/* remove the newline */
#else
			free((char *)e_line); e_line = NULL;
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


		strlcpy(prompt_context, "EDIT value", sizeof(prompt_context));

		/* if we edit an existing entry, push the current value to the edit buffer */
		value = xmlGetProp(db_node, BAD_CAST "value");

#ifndef _READLINE
		el_push(e, (const char *)value);
		xmlFree(value); value = NULL;

		e_line = el_gets(e, &e_count);

		/* re-enable history */
		if (el_set(e, EL_HIST, history, eh) != 0) {
			perror("el_set(EL_HIST)");
		}
#else
		_rl_helper_var = value;
		rl_pre_input_hook = (rl_hook_func_t *)_rl_push_buffer;
		e_line = readline(prompt_str());
		rl_pre_input_hook = NULL;
#endif
		if (e_line) {
			value_rR = xmlStrdup(BAD_CAST e_line); malloc_check(value_rR);
#ifndef _READLINE
			value_rR[xmlStrlen(value_rR) - 1] = '\0';	/* remove the newline */
#else
			free((char *)e_line); e_line = NULL;
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

		modified = malloc(TIME_MAXLEN); malloc_check(modified);
		snprintf(modified, TIME_MAXLEN, "%d", (int)time(NULL));

		xmlSetProp(db_node, BAD_CAST "name", key);
		xmlSetProp(db_node, BAD_CAST "value", value);
		xmlSetProp(db_node, BAD_CAST "modified", BAD_CAST modified);

		/* Update the keychain's modified timestamp */
		xmlSetProp(keychain, BAD_CAST "modified", BAD_CAST modified);

		printf("Modified key: %lu. %s\n", idx, key);

		xmlFree(key); key = NULL;
		xmlFree(value_rR); value_rR = NULL;
		xmlFree(value); value = NULL;
		free(modified); modified = NULL;


		strlcpy(prompt_context, "", sizeof(prompt_context));

		db_params.dirty = 1;
	} else
		puts("Invalid index!");
} /* cmd_edit() */
