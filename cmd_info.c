/*
 * Copyright (c) 2011, 2012, 2013 LEVAI Daniel
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


#include "common.h"
#include "commands.h"


void
cmd_info(const char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*key = NULL, *created = NULL, *modified = NULL;

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

		created = xmlGetProp(db_node, BAD_CAST "created");
		if (created) {
			/* TODO Convert timestamp to calendar date */
		} else
			created = xmlStrdup(BAD_CAST "Not defined.");

		modified = xmlGetProp(db_node, BAD_CAST "modified");
		if (modified) {
			/* TODO Convert timestamp to calendar date */
		} else
			modified = xmlStrdup(BAD_CAST "Not defined.");

		printf("Name: %s\n", key);
		printf("Created: %s\n", created);
		printf("Modified: %s\n", modified);

		xmlFree(key); key = NULL;
		xmlFree(created); created = NULL;
		xmlFree(modified); modified = NULL;
	} else
		puts("invalid index!");
} /* cmd_info() */
