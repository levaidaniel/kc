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
	xmlNodePtr	db_node = NULL;
	xmlChar		*pattern_locale = NULL, *pattern = NULL, *key_locale = NULL, *key = NULL;
	const xmlChar	*search = NULL;

	char		chain = 0;
	int		hits = 0, idx = 0;


	if (strncmp(e_line, "csearch", 7) == 0)
		chain = 1;
	else
		chain = 0;

	pattern_locale = BAD_CAST strtok((char *)e_line, " ");	/* assign the command's parameter */
	if (!pattern_locale) {
		puts(commands->usage);
		return;
	}

	pattern = convert_utf8(pattern_locale, 0);

	if (chain)
		db_node = keychain->parent->children;
	else 
		db_node = keychain->children;

	while (db_node) {
		if (db_node->type == XML_ELEMENT_NODE) {	// skip the non element nodes
			if (chain)
				key = xmlStrdup(xmlGetProp(db_node, BAD_CAST "name"));	// search for keychains
			else
				key = xmlNodeGetContent(db_node->children);	// search for keys in the current keychain

			if (debug)
				printf("name=%s", key);

			search = xmlStrstr(key, pattern);
			if (search) {
				if (debug)
					printf(" <=== hit\n");

				hits++;

				printf("%d. ", idx);    // prefix the name with the index number
				key_locale = convert_utf8(key, 1);
				printf("%s\n", key_locale);    // this is the name of the entry
				free(key_locale); key_locale = NULL;
			} else
				if (debug)
					puts("");

			xmlFree(key); key = NULL;

			idx++;
		}

		db_node = db_node->next;
	}

	if (!hits)
		printf("'%s' not found.\n", pattern_locale);

	free(pattern);
} /* cmd_search() */
