/*
 * Copyright (c) 2011-2018 LEVAI Daniel
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


extern xmlDocPtr	db;
extern xmlNodePtr	keychain;

#ifndef _READLINE
extern EditLine		*e;
#endif

extern char		batchmode;


void
cmd_near(const char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*key = NULL;

	unsigned long int	idx = 0, context = 1, i = 0;
	char			*line = NULL, *cmd = NULL, *inv = NULL;


	line = strdup(e_line); malloc_check(line);


	cmd = strtok(line, " ");
	if (!cmd) {
		puts(commands->usage);

		free(line); line = NULL;
		return;
	}


	cmd = strtok(NULL, " ");	/* first parameter, the index number */
	if (cmd) {
		errno = 0;
		idx = strtoul((const char *)cmd, &inv, 10);
		if (inv[0] != '\0'  ||  errno != 0  ||  cmd[0] == '-') {
			puts(commands->usage);

			free(line); line = NULL;
			return;
		}
	}
	db_node = find_key(idx);
	if (!db_node) {
		puts("Invalid index!");

		free(line); line = NULL;
		return;
	}


	cmd = strtok(NULL, " ");	/* second, optional parameter, the context number */
	if (cmd) {
		errno = 0;
		context = strtoul((const char *)cmd, &inv, 10);
		if (inv[0] != '\0'  ||  errno != 0  ||  cmd[0] == '-') {
			puts(commands->usage);

			free(line); line = NULL;
			return;
		}
	}


	free(line); line = NULL;


	db_node = keychain->children;

	/* Before ... */
	while (db_node  &&  i < idx) {
		/* We only care about ELEMENT nodes */
		if (db_node->type != XML_ELEMENT_NODE) {
			db_node = db_node->next;
			continue;
		}

		if (i >= idx - (context > idx ? idx : context)) {
			key = xmlGetProp(db_node, BAD_CAST "name");
			printf(" %lu. %s\n", i, key);
			xmlFree(key); key = NULL;
		}

		i++;

		db_node = db_node->next;
	}


	/* The requested key ... */
	while (db_node  &&  db_node->type != XML_ELEMENT_NODE)
		db_node = db_node->next;	/* The very next element node */


	/* Just to be safe... */
	if (!db_node)
		return;


	key = xmlGetProp(db_node, BAD_CAST "name");
	printf("=%lu. %s\n", i, key);
	xmlFree(key); key = NULL;


	i++;
	db_node = db_node->next;

	/* After ... */
	while (db_node) {
		/* We only care about ELEMENT nodes */
		if (db_node->type != XML_ELEMENT_NODE) {
			db_node = db_node->next;
			continue;
		}

		if (i <= idx + context) {
			key = xmlGetProp(db_node, BAD_CAST "name");
			printf(" %lu. %s\n", i, key);
			xmlFree(key); key = NULL;
		}

		i++;

		db_node = db_node->next;
	}
} /* cmd_near() */
