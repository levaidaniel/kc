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
cmd_del(char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL, db_node_prev = NULL;
	xmlChar		*key = NULL;

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
		key = xmlGetProp(db_node, BAD_CAST "name");

		db_node_prev = db_node->prev;
		xmlUnlinkNode(db_node_prev);	// remove the adjacent 'text' node, which is the indent and newline
		xmlFreeNode(db_node_prev);

		xmlUnlinkNode(db_node);
		xmlFreeNode(db_node);

		printf("'%s' deleted\n", key);
		xmlFree(key); key = NULL;

		dirty = 1;
	} else
		puts("invalid index!");
} /* cmd_del() */
