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

#ifndef _READLINE
extern EditLine		*e;
extern History		*eh;
#endif


void
cmd_cdel(const char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL, db_node_tmp = NULL;
	xmlChar		*cname = NULL;

	char		*cmd = NULL, name = 0;
	char		*line = NULL;

#ifndef _READLINE
	int		e_count = 0;
#endif


	line = strdup(e_line);

	cmd = strtok(line, " ");		/* get the command name */
	if (strncmp(cmd, "cc", 2) == 0)
		name = 1;			/* force to search for the keychain's name */

	cname = BAD_CAST strtok(NULL, " ");	/* assign the command's parameter */
	if (!cname) {
		puts(commands->usage);
		free(line); line = NULL;
		return;
	}

	db_node = find_keychain(cname, name);
	free(line); line = NULL;
	if (db_node) {
		/* don't allow to delete the current keychain. this saves us trouble. */
		if (	xmlStrcmp(	xmlGetProp(keychain, BAD_CAST "name"),
					xmlGetProp(db_node, BAD_CAST "name")) == 0) {

			puts("Can not delete the current keychain!");
		} else {
#ifndef _READLINE
			/* disable history temporarily */
			if (el_set(e, EL_HIST, history, NULL) != 0) {
				perror("el_set(EL_HIST)");
			}
			/* clear the prompt temporarily */
			if (el_set(e, EL_PROMPT, el_prompt_null) != 0) {
				perror("el_set(EL_PROMPT)");
			}
#endif
			cname = xmlGetProp(db_node, BAD_CAST "name");

			printf("Do you really want to delete '%s'? <yes/no> ", cname);

#ifdef _READLINE
			rl_redisplay();
#endif

#ifndef _READLINE
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
			e_line = readline("");
#endif
			if (!e_line) {
#ifndef _READLINE
				el_reset(e);
#endif
				return;
			}

			if (strncmp(e_line, "yes", 3) == 0) {
				db_node_tmp = db_node->prev;
				xmlUnlinkNode(db_node_tmp);
				xmlFreeNode(db_node_tmp);

				xmlUnlinkNode(db_node);
				xmlFreeNode(db_node);

				printf("Deleted keychain: %s\n", cname);
				puts("Keychain indices have been changed. Make sure to 'clist', before using them again!");

				dirty = 1;
			}
			xmlFree(cname); cname = NULL;
		}
	} else {
		printf("'%s' keychain not found.\n", cname);
	}

} /* cmd_cdel() */
