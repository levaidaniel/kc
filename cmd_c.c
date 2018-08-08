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
cmd_c(const char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*cname = NULL;

	char		*cmd = NULL, name = 0;
	char		*line = NULL;


	line = strdup(e_line); malloc_check(line);

	cmd = strtok(line, " ");		/* get the command name */
	if (strncmp(cmd, "cc", 2) == 0)
		name = 1;			/* force to search for the keychain's name */

	cname = BAD_CAST strtok(NULL, " ");		/* assign the command's parameter */
	if (!cname) {
		puts(commands->usage);
		free(line); line = NULL;
		return;
	}

	db_node = find_keychain(cname, name);
	if (db_node)
		keychain = db_node;
	else
		puts("Keychain not found.");

	free(line); line = NULL;
} /* cmd_c() */
