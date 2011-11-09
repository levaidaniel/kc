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
cmd_edit(char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL, db_node_new = NULL;
	xmlChar		*key = NULL, *value_nl = NULL, *value = NULL;

#ifndef _READLINE
	int		e_count = 0;
#endif
	int		idx = 0;


	if (sscanf(e_line, "%*s %d", &idx) <= 0) {
		puts(commands->usage);
		return;
	}
	if (idx < 0) {
		puts(commands->usage);
		return;
	}

	db_node = find_key(idx);
	if (db_node) {
#ifndef _READLINE
		// disable history temporarily
		if (el_set(e, EL_HIST, history, NULL) != 0) {
			perror("el_set(EL_HIST)");
		}
#endif

		strlcpy(prompt_context, "EDIT key", sizeof(prompt_context));

		// if we edit an existing entry, push the current value to the edit buffer
		key = xmlGetProp(db_node, BAD_CAST "name");
#ifdef _READLINE
		_rl_helper_var = key;
#endif

#ifndef _READLINE
		el_push(e, (const char *)key);
		xmlFree(key); key = NULL;

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
			el_reset(e);
			return;
		} else {
			key = xmlStrdup(BAD_CAST e_line);
		}


		strlcpy(prompt_context, "EDIT value", sizeof(prompt_context));

		// if we edit an existing entry, push the current value to the edit buffer
		value_nl = xmlGetProp(db_node, BAD_CAST "value");
		value = parse_newlines(value_nl, 1);
		xmlFree(value_nl); value_nl = NULL;
#ifdef _READLINE
		_rl_helper_var = value;
#endif

#ifndef _READLINE
		el_push(e, (const char *)value);
		free(value); value = NULL;

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
			el_reset(e);
			xmlFree(key); key = NULL;
			return;
		} else {
			value = BAD_CAST e_line;
		}

		value = parse_newlines(value, 0);


		db_node_new = xmlNewChild(keychain, NULL, BAD_CAST "key", NULL);
		xmlNewProp(db_node_new, BAD_CAST "name", key);
		xmlNewProp(db_node_new, BAD_CAST "value", value);
		xmlFree(key); key = NULL;
		free(value); value = NULL;

		db_node = xmlReplaceNode(db_node, db_node_new);
		xmlFreeNode(db_node);


#ifndef _READLINE
		// re-enable history
		if (el_set(e, EL_HIST, history, eh) != 0) {
			perror("el_set(EL_HIST)");
		}
#endif

		strlcpy(prompt_context, "", sizeof(prompt_context));

		dirty = 1;
	} else
		puts("invalid index!");
} /* cmd_edit() */
