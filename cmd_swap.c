/*
 * Copyright (c) 2011, 2012, 2013 LEVAI Daniel
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


#include "common.h"
#include "commands.h"


extern xmlNodePtr	keychain;
extern char		dirty;


void
cmd_swap(const char *e_line, command *commands)
{
	xmlNodePtr	key_src = NULL, key_dst = NULL, key_tmp_src = NULL, key_tmp_dst = NULL;

	char		*modified = NULL;
	int		idx_src = 0, idx_dst = 0;


	if (sscanf(e_line, "%*s %d %d", &idx_src, &idx_dst) < 2) {
		puts(commands->usage);
		return;
	}
	if (idx_src < 0  ||  idx_dst < 0) {
		puts(commands->usage);
		return;
	}

	key_src = find_key(idx_src);
	if (!key_src) {
		puts("Invalid source index!");
		return;
	}
	key_dst = find_key(idx_dst);
	if (!key_dst) {
		puts("Invalid destination index!");
		return;
	}

	/* duplicate the source key */
	key_tmp_src = xmlCopyNode(key_src, 2);
	if (!key_tmp_src) {
		puts("Error duplicating source entry!");

		if (getenv("KC_DEBUG"))
			puts("xmlCopyNode() error!");

		return;
	}

	/* duplicate the destination key */
	key_tmp_dst = xmlCopyNode(key_dst, 2);
	if (!key_tmp_dst) {
		puts("Error duplicating destination entry!");

		if (getenv("KC_DEBUG"))
			puts("xmlCopyNode() error!");

		return;
	}


	/* We need two key_tmp_* variables, because xmlReplaceNode() unlinks both its parameters */
	xmlReplaceNode(key_src, key_tmp_dst);
	xmlFreeNode(key_src);

	xmlReplaceNode(key_dst, key_tmp_src);
	xmlFreeNode(key_dst);


	/* Update the current keychain's modified timestamp */
	modified = malloc(TIME_MAXLEN); malloc_check(modified);
	snprintf(modified, TIME_MAXLEN, "%d", (int)time(NULL));

	xmlSetProp(keychain, BAD_CAST "modified", BAD_CAST modified);


	printf("Key %d was swapped with %d\n", idx_src, idx_dst);

	dirty = 1;
} /* cmd_swap() */
