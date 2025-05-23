/*
 * Copyright (c) 2011-2025 LEVAI Daniel
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


void
cmd_clist(const char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*name = NULL, *description = NULL;

	unsigned long int	idx = 0;


	/* these are unused in this function */
	e_line = NULL;
	commands = NULL;

	db_node = keychain->parent->children;

	if (getenv("KC_DEBUG"))
		printf("%s(): #BEGIN\n", __func__);

	while (db_node  &&  idx < ITEMS_MAX) {
		if (db_node->type == XML_ELEMENT_NODE) {	/* we only care about ELEMENT nodes */
			name = xmlGetProp(db_node, BAD_CAST "name");
			description = xmlGetProp(db_node, BAD_CAST "description");

			printf("%lu. %s [%s]\n", idx++, name, description ? description : BAD_CAST "");

			xmlFree(name); name = NULL;
			xmlFree(description); name = NULL;
		}

		db_node = db_node->next;
	}

	if (getenv("KC_DEBUG"))
		printf("%s(): #END\n", __func__);
} /* cmd_clist() */
