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


extern xmlNodePtr	keychain;


void
cmd_info(const char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*name = NULL, *created = NULL, *modified = NULL, *description = NULL;

	char			key = 0;
	char			*line = NULL, *cmd = NULL, *inv = NULL;
	unsigned long int	idx = 0;
	time_t			created_time = 0, modified_time = 0;


	line = strdup(e_line); malloc_check(line);


	cmd = strtok(line, " ");
	if (!cmd) {
		puts(commands->usage);

		free(line); line = NULL;
		return;
	}


	cmd = strtok(NULL, " ");	/* first optional parameter, the index number */
	if (cmd) {
		errno = 0;
		idx = strtoul((const char *)cmd, &inv, 10);
		if (inv[0] != '\0'  ||  errno != 0  ||  cmd[0] == '-') {
			puts(commands->usage);

			free(line); line = NULL;
			return;
		}

		db_node = find_key(idx);
		key = 1;
	} else
		db_node = keychain;



	free(line); line = NULL;


	if (db_node) {
		name = xmlGetProp(db_node, BAD_CAST "name");
		if (key)
			printf("Key name: %s\n", name);
		else
			printf("Keychain name: %s\n", name);
		xmlFree(name); name = NULL;

		if (!key) {
			printf("Description: ");
			description = xmlGetProp(db_node, BAD_CAST "description");
			if (description) {
				printf("%s\n", description);
				xmlFree(description); description = NULL;
			} else
				puts("Not defined.");
		}

		printf("Created: ");
		created = xmlGetProp(db_node, BAD_CAST "created");
		if (created) {
			created_time = atoi((const char *)created);
			printf("%s", ctime(&created_time));
			xmlFree(created); created = NULL;
		} else
			puts("Not defined.");

		printf("Modified: ");
		modified = xmlGetProp(db_node, BAD_CAST "modified");
		if (modified) {
			modified_time = atoi((const char *)modified);
			printf("%s", ctime(&modified_time));
			xmlFree(modified); modified = NULL;
		} else
			puts("Not defined.");
	} else
		puts("Invalid index!");
} /* cmd_info() */
