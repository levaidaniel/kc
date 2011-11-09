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

#ifdef	_HAVE_PCRE
#include <pcre.h>
#endif

#include "common.h"
#include "commands.h"


extern xmlNodePtr	keychain;


void
cmd_searchre(char *e_line, command *commands)
{
#ifdef	_HAVE_PCRE
	xmlNodePtr	db_node = NULL, search_keychain = NULL;
	xmlChar		*key = NULL;

	pcre		*re = NULL;
	pcre_extra	*re_study = NULL;
	const char	*error = NULL;
	int		erroffset = 0;
	int		ovector[30];

	const char	*pattern = NULL;
	char		*line = NULL;
	char		chain = 0, searchall = 0;
	int		hits = 0, idx = 0;


	line = strdup(e_line);

	if (strncmp(line, "*", 1) == 0) {
		searchall = 1;
		line++;
	}
	if (strncmp(line, "c", 1) == 0)
		chain = 1;


	if (chain)
		pattern = line + 2;	// remove the 'c/' from the line. the remaining part is the pattern
	else
		pattern = line + 1;	// remove the '/'(slash) from the line. the remaining part is the pattern

	if (!pattern) {
		puts(commands->usage);
		free(line);
		return;
	}
	if (strlen(pattern) <= 0) {
		puts(commands->usage);
		free(line);
		return;
	}


	re = pcre_compile(pattern, PCRE_UTF8, &error, &erroffset, NULL);
	if (!re) {
		printf("error in pattern at %d: (%s)\n", erroffset, error);
		free(line);
		return;
	}

	re_study = pcre_study(re, 0, &error);
	if (!re_study) {
		if (debug)
			puts("pcre_study(): could not optimize regexp");

		if (!error) {
			perror("pcre_study()");
			free(line);
			return;
		}
	}


	search_keychain = keychain;
	while (search_keychain) {
		if (search_keychain->type != XML_ELEMENT_NODE) {	// skip the non element nodes
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
			if (db_node->type != XML_ELEMENT_NODE) {	// skip the non element nodes
				db_node = db_node->next;
				continue;
			}

			key = xmlGetProp(db_node, BAD_CAST "name");

			if (debug)
				printf("name=%s", key);

			if (pcre_exec(re, re_study, (const char *)key, xmlStrlen(key), 0, 0, ovector, 30) >= 0) {
				if (debug)
					printf(" <=== hit\n");

				hits++;

				if (searchall)
					printf("%s%% ", xmlGetProp(search_keychain, BAD_CAST "name"));  // prefix the name with the keychain name

				printf("%d. ", idx);	// prefix the name with the index number
				printf("%s\n", key);	// this is the name of the entry
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
			search_keychain = NULL;         /* force the quit from the loop */
	}

	if (!hits)
		printf("'%s' not found.\n", pattern);

	free(line);
#else
	puts("regexp support was not compiled in.");
#endif
} /* cmd_searchre() */
