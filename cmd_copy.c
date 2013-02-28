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


#include "common.h"
#include "commands.h"


extern xmlNodePtr	keychain;
extern char		dirty;


void
cmd_copy(const char *e_line, command *commands)
{
	xmlNodePtr	key = NULL, key_new = NULL, keychain_dest = NULL, db_node_prev = NULL;
	xmlChar		*cname = NULL, *name = NULL;

	char		*line = NULL, *modified = NULL;

	char		*cmd = NULL, *idx_str = NULL, move = 0;
	int		idx = 0;


	line = strdup(e_line);

	cmd = strtok(line, " ");		/* get the command name */
	if (	strcmp(cmd, "move") == 0  ||
		strcmp(cmd, "mv") == 0)
		move = 1;

	idx_str = strtok(NULL, " ");
	cname = BAD_CAST strtok(NULL, " ");	/* assign the command's parameter */
	if (!cname  ||  !idx_str) {
		puts(commands->usage);
		free(line); line = NULL;
		return;
	}

	if (sscanf(idx_str, "%d", &idx) <= 0) {
		puts(commands->usage);
		free(line); line = NULL;
		return;
	}
	if (idx < 0) {
		puts(commands->usage);
		free(line); line = NULL;
		return;
	}

	keychain_dest = find_keychain(cname, 0);
	if (!keychain_dest) {
		puts("Keychain not found.");
		free(line); line = NULL;
		return;
	}

	key = find_key(idx);
	if (!key) {
		puts("Invalid index!");
		free(line); line = NULL;
		return;
	} else {
		/* duplicate the key which is to be copied */
		key_new = xmlCopyNode(key, 2);
		if (!key_new) {
			puts("Error copying entry!");

			if (getenv("KC_DEBUG"))
				puts("xmlCopyNode() error!");

			free(line); line = NULL;
			return;
		}

		if (move) {	/* unlink from the source keychain */
			/* remove the adjacent 'text' node, which is the indent and newline */
			db_node_prev = key->prev;
			xmlUnlinkNode(db_node_prev);
			xmlFreeNode(db_node_prev);

			xmlUnlinkNode(key);	/* remove the node itself */
		}

		/* add the new entry to the destination keychain */
		xmlAddChild(keychain_dest, xmlNewText(BAD_CAST "\t"));	/* make the XML document prettttyyy */
		xmlAddChild(keychain_dest, key_new);
		xmlAddChild(keychain_dest, xmlNewText(BAD_CAST "\n\t"));	/* make the XML document prettttyyy */


		/* Update the current keychain's modified timestamp */
		modified = malloc(TIME_MAXLEN); malloc_check(modified);
		snprintf(modified, TIME_MAXLEN, "%d", (int)time(NULL));

		xmlSetProp(keychain, BAD_CAST "modified", BAD_CAST modified);

		/* Update the destination keychain's modified timestamp */
		xmlSetProp(keychain_dest, BAD_CAST "modified", BAD_CAST modified);


		name = xmlGetProp(key, BAD_CAST "name");
		printf("Key '%d. %s' was %s to keychain: %s\n", idx, name, move ? "moved" : "copied", cname);
		xmlFree(name); name = NULL;

		dirty = 1;
	}

	free(line); line = NULL;
} /* cmd_copy() */
