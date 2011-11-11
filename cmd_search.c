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
cmd_search(char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL, search_keychain = NULL;
	xmlChar		*pattern = NULL, *key = NULL;
	const xmlChar	*search = NULL;

	char		*cmd = NULL;
	char		chain = 0, searchall = 0;
	int		hits = 0, idx = 0;


	cmd = strtok(e_line, " ");		/* get the command name */
	if (strncmp(cmd, "*", 1) == 0) {
		searchall = 1;
		cmd++;
	}

	if (strncmp(cmd, "c", 1) == 0)
		chain = 1;

	
	pattern = BAD_CAST strtok(NULL, " ");	/* assign the command's parameter */
	if (!pattern) {
		puts(commands->usage);
		return;
	}


	search_keychain = keychain;
	while (search_keychain) {
		if (search_keychain->type != XML_ELEMENT_NODE) {	/* skip the non element nodes */
			search_keychain = search_keychain->next;
			continue;
		}

		/* rewind to the first item, be it a keychain or a key */
		if (chain)
			db_node = search_keychain->parent->children;
		else
			db_node = search_keychain->children;

		if (debug)
			printf("searching for: '%s' in '%s' chain\n", pattern, xmlGetProp(search_keychain, BAD_CAST "name"));

		idx = 0;
		while (db_node) {
			if (db_node->type != XML_ELEMENT_NODE) {	/* skip the non element nodes */
				db_node = db_node->next;
				continue;
			}

			key = xmlGetProp(db_node, BAD_CAST "name");

			if (debug)
				printf("name=%s", key);

			search = xmlStrstr(key, pattern);
			if (search) {
				if (debug)
					printf(" <=== hit\n");

				hits++;

				if (searchall)
					printf("%s%% ", xmlGetProp(search_keychain, BAD_CAST "name"));	/* prefix the name with the keychain name */

				printf("%d. ", idx);	/* prefix the name with the index number */
				printf("%s\n", key);	/* this is the name of the entry */
				xmlFree(key); key = NULL;
			} else
				if (debug)
					puts("");

			idx++;

			db_node = db_node->next;
		}

		if (searchall)
			search_keychain = search_keychain->next;
		else
			search_keychain = NULL;		/* force the quit from the loop */
	}

	if (!hits)
		printf("'%s' not found.\n", pattern);
} /* cmd_search() */
