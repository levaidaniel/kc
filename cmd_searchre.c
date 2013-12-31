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


#ifdef	_HAVE_PCRE
#include <pcre.h>
#endif

#include "common.h"
#include "commands.h"


extern xmlNodePtr	keychain;

extern char		batchmode;


void
cmd_searchre(const char *e_line, command *commands)
{
#ifdef	_HAVE_PCRE
	xmlNodePtr	db_node = NULL, search_keychain = NULL;
	xmlChar		*key = NULL, *cname = NULL;

	pcre		*re = NULL;
	pcre_extra	*re_study = NULL;
	const char	*error = NULL;
	int		erroffset = 0;
	int		ovector[30];

	const char	*pattern = NULL;
	char		chain = 0, searchall = 0, searchinv = 0, icase = 0;
	long int	idx = 0;
	int		search = -1, hits = 0, offset = 0;


	/* Command name */
	if (strncmp(e_line + offset, "c", 1) == 0) {
		chain = 1;
		offset++;
	}

	/* Jump over the command that is a '/'(slash). */
	offset++;

	/* Modifiers */
	for (; (offset < strlen(e_line)  &&  e_line[offset] != ' '); offset++)
		switch (e_line[offset]) {
			case '!':
				searchinv = 1;
				break;
			case 'i':
				icase = 1;
				break;
			case '*':
				/* this doesn't make sense with keychain searching */
				if (chain) {
					puts(commands->usage);
					return;
				} else
					searchall = 1;
				break;
		}

	/* the occasional space after the command */
	offset++;
	if (offset >= strlen(e_line)) {
		puts(commands->usage);
		return;
	}

	pattern = e_line + offset;
	if (!pattern) {
		puts(commands->usage);
		return;
	}


	re = pcre_compile(pattern, PCRE_UTF8 | (icase ? PCRE_CASELESS : 0), &error, &erroffset, NULL);
	if (!re) {
		printf("Error in pattern at %d: (%s)\n", erroffset, error);
		return;
	}

	re_study = pcre_study(re, 0, &error);
	if (!re_study) {
		if (getenv("KC_DEBUG"))
			puts("pcre_study(): could not optimize regexp");

		if (!error) {
			perror("pcre_study()");
			return;
		}
	}


	if (searchall)
		/* if *search'ing, start from the first keychain */
		search_keychain = keychain->parent->children;
	else
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

		if (getenv("KC_DEBUG"))
			printf("searching for: '%s' in '%s' chain\n", pattern, xmlGetProp(search_keychain, BAD_CAST "name"));

		idx = 0;
		while (db_node) {
			if (db_node->type != XML_ELEMENT_NODE) {	/* skip the non element nodes */
				db_node = db_node->next;
				continue;
			}

			key = xmlGetProp(db_node, BAD_CAST "name");

			if (getenv("KC_DEBUG"))
				printf("name=%s", key);

			search = pcre_exec(re, re_study, (const char *)key, xmlStrlen(key), 0, 0, ovector, 30);
			/* poor man's XOR: */
			if (((search >= 0)  ||  searchinv)  &&  !((search >= 0)  &&  searchinv)) {
				if (getenv("KC_DEBUG"))
					printf(" <=== hit\n");

				hits++;

				if (searchall) {
					cname = xmlGetProp(search_keychain, BAD_CAST "name");
					printf("%s%% ", cname);	/* prefix the name with the keychain name */
					xmlFree(cname); cname = NULL;
				}

				printf("%ld. ", idx);	/* prefix the name with the index number */
				printf("%s\n", key);	/* this is the name of the entry */
			} else
				if (getenv("KC_DEBUG"))
					puts("");


			xmlFree(key); key = NULL;

			idx++;

			db_node = db_node->next;
		}

		if (searchall)
			search_keychain = search_keychain->next;
		else
			search_keychain = NULL;		/* force the quit from the loop */
	}

	pcre_free(re);
	pcre_free_study(re_study);

	if (hits > 0) {
		if (hits > 5  &&  !batchmode)
			printf("\n %d keys were found.\n", hits);
	} else {
		printf("'%s' was not found.\n", pattern);
	}
#else
	puts("PCRE - regular expression - support was not compiled in!");
#endif
} /* cmd_searchre() */
