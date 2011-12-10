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


void
cmd_copy(const char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL, db_node_c = NULL, db_node_prev = NULL;
	xmlChar		*cname = NULL;

	char		*line = NULL;

	char		*cmd = NULL, *idx_str = NULL, move = 0;
	int		idx = 0;


	line = strdup(e_line);

	cmd = strtok(line, " ");		/* get the command name */
	if (strcmp(cmd, "move") == 0)
		move = 1;

	idx_str = strtok(NULL, " ");
	cname = BAD_CAST strtok(NULL, " ");	/* assign the command's parameter */
	if (!cname  ||  !idx_str) {
		puts(commands->usage);
		free(line); line = NULL;
		return;
	}

	if (sscanf(idx_str, "%d", &idx) <= 0) {
		puts(commands->usage);
		free(line); line = NULL;
		return;
	}
	if (idx < 0) {
		puts(commands->usage);
		free(line); line = NULL;
		return;
	}

	db_node_c = find_keychain(cname);
	if (!db_node_c) {
		puts("keychain not found.");
		free(line); line = NULL;
		return;
	}

	db_node = find_key(idx);
	if (!db_node) {
		puts("invalid index!");
		free(line); line = NULL;
		return;
	} else {
		if (move) {	/* unlink from the source keychain */
			/* remove the adjacent 'text' node, which is the indent and newline */
			db_node_prev = db_node->prev;
			xmlUnlinkNode(db_node_prev);
			xmlFreeNode(db_node_prev);

			/* remove the node itself */
			xmlUnlinkNode(db_node);
		}

		/* add the new entry to the destination keychain */
		/* make the XML document prettttyyy */
		xmlAddChild(db_node_c, xmlNewText(BAD_CAST "\t"));

		xmlAddChild(db_node_c, db_node);

		/* make the XML document prettttyyy */
		xmlAddChild(db_node_c, xmlNewText(BAD_CAST "\n\t"));


		dirty = 1;
	}

	free(line); line = NULL;
} /* cmd_copy() */
