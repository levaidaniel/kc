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


void
cmd_clist(const char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*cname = NULL;

	int		idx = 0;


	db_node = keychain->parent->children;

	if (debug)
		printf("#BEGIN\n");

	while (db_node) {
		if (db_node->type == XML_ELEMENT_NODE) {	/* we only care about ELEMENT nodes */
			cname = xmlGetProp(db_node, BAD_CAST "name");
			printf("%d. %s\n", idx++, cname);
			xmlFree(cname); cname = NULL;
		}

		db_node = db_node->next;
	}

	if (debug)
		printf("#END\n");
} /* cmd_clist() */
