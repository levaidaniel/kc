/*
 * Copyright (c) 2011-2020 LEVAI Daniel
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
	int		i = 0;


	line = strdup(e_line); malloc_check(line);

	/* Search for the first space in the command line, to see if
	 * a key name was specified. If it was, the space will be right
	 * after the command's name.
	 */
	for (i = 0; line[i] != ' '  &&  line[i] != '\0'; i++) {}

	/* Search for the first non-space character after the first space
	 * after the command name; this will be start of the key's name
	 * (if it's been specified).
	 * If no keyname was specified, then it will be a zero sized string,
	 * and we check for that.
	 */
	for (; line[i] == ' '; i++) {}

	cmd = strtok(line, " ");		/* get the command name */
	if (strncmp(cmd, "cc", 2) == 0)
		name = 1;			/* force to search for the keychain's name */

	cname = xmlStrdup(BAD_CAST &line[i]); malloc_check(cname);	/* assign the command's parameter */
	free(line); line = NULL;

	if (xmlStrlen(cname) <= 0) {	/* if we didn't get a keychain name or number as a parameter */
		puts(commands->usage);

		xmlFree(cname); cname = NULL;
		return;
	}

	db_node = find_keychain(cname, name);
	if (db_node)
		keychain = db_node;
	else
		puts("Keychain not found.");

	xmlFree(cname); cname = NULL;
} /* cmd_c() */
