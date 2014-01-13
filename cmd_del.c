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

#ifndef _READLINE
extern EditLine		*e;
extern History		*eh;
#endif


void
cmd_del(const char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL, db_node_prev = NULL;
	xmlChar		*key = NULL;

	char			*modified = NULL;
	char			*line = NULL, *cmd = NULL, *inv = NULL;
	unsigned long int	idx = 0;

#ifndef _READLINE
	int		e_count = 0;
#endif


	line = strdup(e_line);


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
		key = xmlGetProp(db_node, BAD_CAST "name");

		printf("Do you really want to delete '%s'? <yes/no> ", key);

#ifndef _READLINE
		/* disable history temporarily */
		if (el_set(e, EL_HIST, history, NULL) != 0) {
			perror("el_set(EL_HIST)");
		}
		/* clear the prompt temporarily */
		if (el_set(e, EL_PROMPT, el_prompt_null) != 0) {
			perror("el_set(EL_PROMPT)");
		}

		e_line = el_gets(e, &e_count);

		/* re-enable the default prompt */
		if (el_set(e, EL_PROMPT, prompt_str) != 0) {
			perror("el_set(EL_PROMPT)");
		}
		/* re-enable history */
		if (el_set(e, EL_HIST, history, eh) != 0) {
			perror("el_set(EL_HIST)");
		}
#else
		rl_redisplay();
		e_line = readline("");
#endif

		if (!e_line) {
#ifndef _READLINE
			el_reset(e);
#endif
			return;
		}

		if (strncmp(e_line, "yes", 3) == 0) {
			db_node_prev = db_node->prev;
			xmlUnlinkNode(db_node_prev);	/* remove the adjacent 'text' node, which is the indent and newline */
			xmlFreeNode(db_node_prev);

			xmlUnlinkNode(db_node);
			xmlFreeNode(db_node);

			printf("Deleted key: %lu. %s\n", idx, key);
			puts("Key indices have been changed. Make sure to 'list', before using them again!");

			/* Update the keychain's modified timestamp */
			modified = malloc(TIME_MAXLEN); malloc_check(modified);
			snprintf(modified, TIME_MAXLEN, "%d", (int)time(NULL));
			xmlSetProp(keychain, BAD_CAST "modified", BAD_CAST modified);

			free(modified); modified = NULL;

			db_params.dirty = 1;
		}

#ifdef _READLINE
		free((char *)e_line); e_line = NULL;
#endif
		xmlFree(key); key = NULL;
	} else
		puts("Invalid index!");
} /* cmd_del() */
