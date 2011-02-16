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
cmd_searchre(EditLine *e, ...)
{
#ifdef	_HAVE_PCRE
	va_list		ap;

	xmlNodePtr	db_node = NULL;
	xmlChar		*pattern_locale = NULL, *key_locale = NULL, *pattern = NULL, *key = NULL;

	command		*commands = NULL;

	pcre		*re = NULL;
	pcre_extra	*re_study = NULL;
	const char	*error = NULL;
	int		erroffset = 0;
	int		ovector[30];

	char		*line = NULL, *cmd = NULL;
	char		chain = 0;
	int		hits = 0, idx = 0;


	va_start(ap, e);

	line = va_arg(ap, char *);
	line[strlen(line) - 1] = '\0';		// remove the newline character from the end

	va_arg(ap, History *);
	va_arg(ap, BIO *);
	commands = va_arg(ap, command *);

	va_end(ap);

	cmd = strtok(line, " ");
	if (strncmp(cmd, "c/", 2) == 0)
		chain = 1;
	else
		chain = 0;

	if (chain)
		pattern_locale = BAD_CAST line + 2;	// remove the 'c/' from the line. the remaining part is the pattern
	else
		pattern_locale = BAD_CAST line + 1;	// remove the '/'(slash) from the line. the remaining part is the pattern

	if (!pattern_locale) {
		puts(commands->usage);
		return;
	}

	pattern = convert_utf8(pattern_locale, 0);


	re = pcre_compile((const char *)pattern, PCRE_UTF8, &error, &erroffset, NULL);
	if (!re) {
		printf("error in pattern at %d: (%s)\n", erroffset, error);
		free(pattern);
		return;
	}

	re_study = pcre_study(re, 0, &error);
	if (!re_study) {
		if (debug)
			puts("pcre_study(): could not optimize regexp");

		if (!error) {
			perror("pcre_study()");
			return;
		}
	}


	if (chain)
		db_node = keychain->parent->children;
	else
		db_node = keychain->children;

	while (db_node) {
		if (db_node->type == XML_ELEMENT_NODE) {	// skip the non element nodes
			if (chain)
				key = xmlStrdup(xmlGetProp(db_node, BAD_CAST "name"));
			else
				key = xmlNodeGetContent(db_node->children);

			if (debug)
				printf("name=%s", key);

			if (pcre_exec(re, re_study, (const char *)key, xmlStrlen(key), 0, 0, ovector, 30) >= 0) {
				if (debug)
					printf(" <=== hit\n");

				hits++;

				printf("%d. ", idx);	// prefix the name with the index number 
				key_locale = convert_utf8(key, 1);
				printf("%s\n", key_locale);	// this is the name of the entry
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
#else
	puts("regexp support was not compiled in.");
#endif
} /* cmd_searchre() */
