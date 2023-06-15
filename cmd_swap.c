/*
 * Copyright (c) 2011-2023 LEVAI Daniel
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


char	swap_keys(xmlNodePtr *, xmlNodePtr *);


extern db_parameters	db_params;
extern xmlNodePtr	keychain;


void
cmd_swap(const char *e_line, command *commands)
{
	xmlNodePtr	key_src = NULL, key_dst = NULL, key_finish = NULL;

	char			*modified = NULL;
	char			insert = 0;
	char			*line = NULL, *cmd = NULL, *inv = NULL;
	unsigned long int	idx_src = 0, idx_dst = 0;


	line = strdup(e_line); malloc_check(line);


	cmd = strtok(line, " ");	/* command name */
	if (!cmd) {
		puts(commands->usage);

		free(line); line = NULL;
		return;
	}

	if (strcmp(cmd, "insert") == 0)
		insert = 1;


	cmd = strtok(NULL, " ");	/* first parameter, the source index number */
	if (!cmd) {
		puts(commands->usage);

		free(line); line = NULL;
		return;
	}

	errno = 0;
	idx_src = strtoul((const char *)cmd, &inv, 10);
	if (inv[0] != '\0'  ||  errno != 0  ||  cmd[0] == '-') {
		puts(commands->usage);

		free(line); line = NULL;
		return;
	}


	cmd = strtok(NULL, " ");	/* second parameter, the destination index number */
	if (!cmd) {
		puts(commands->usage);

		free(line); line = NULL;
		return;
	}

	errno = 0;
	idx_dst = strtoul((const char *)cmd, &inv, 10);
	if (inv[0] != '\0'  ||  errno != 0  ||  cmd[0] == '-') {
		puts(commands->usage);

		free(line); line = NULL;
		return;
	}


	free(line); line = NULL;


	/* No funny stuff!!! :) */
	if (idx_src == idx_dst) {
		puts("The two indices are the same!");
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


	if (!swap_keys(&key_src, &key_dst)) {
		if (insert)
			dprintf(STDERR_FILENO, "ERROR: Error while preparing to insert key!\n");
		else
			dprintf(STDERR_FILENO, "ERROR: Error while trying to swap keys!\n");

		return;
	}


	if (insert) {
		/* Inserting is basically swapping the two indices, then shift the surrounding indices ...
		 *
		 * if (idx_src > idx_dst)
		 * ... from the bottom to the top
		 *
		 * if (idx_src < idx_dst)
		 * ... from the top to the bottom
		 */

		key_finish = key_src;
		key_src = key_dst;

		/* We skip one node; it's the text node for formatting */
		if (idx_src > idx_dst)
			key_dst = key_src->prev->prev;
		else
			key_dst = key_src->next->next;

		while (key_dst) {
			/* finish */
			if (key_dst == key_finish)
				break;

			if (key_dst->type != XML_ELEMENT_NODE) {
				dprintf(STDERR_FILENO, "ERROR: Error while trying to insert key! Please check your key list and reload your database without saving, if necessary!\n");
				if (getenv("KC_DEBUG"))
					printf("%s(): key_dst is non-element node!\n", __func__);

				return;
			}

			if (!swap_keys(&key_src, &key_dst)) {
				dprintf(STDERR_FILENO, "ERROR: Error while trying to insert key! Please check your key list and reload your database without saving, if necessary!\n");
				if (getenv("KC_DEBUG"))
					printf("%s(): swap_key() error while shifting nodes!\n", __func__);

				return;
			}

			if (idx_src > idx_dst)
				key_dst = key_src->prev->prev;
			else
				key_dst = key_src->next->next;
		}
	}

	/* Update the current keychain's modified timestamp */
	modified = malloc(TIME_MAXLEN); malloc_check(modified);
	snprintf(modified, TIME_MAXLEN, "%d", (int)time(NULL));

	xmlSetProp(keychain, BAD_CAST "modified", BAD_CAST modified);

	free(modified); modified = NULL;

	if (insert)
		printf("Key %lu was inserted at %lu\n", idx_src, idx_dst);
	else
		printf("Key %lu was swapped with %lu\n", idx_src, idx_dst);

	puts("Key indices may have been changed. Make sure to 'list', before using them again!");

	db_params.dirty = 1;
} /* cmd_swap() */


char
swap_keys(xmlNodePtr *key_src, xmlNodePtr *key_dst)
{
	xmlNodePtr	key_tmp_src = NULL, key_tmp_dst = NULL;


	/* duplicate the source key */
	key_tmp_src = xmlCopyNode(*key_src, 2);
	if (!key_tmp_src) {
		dprintf(STDERR_FILENO, "ERROR: Error duplicating source entry!\n");

		if (getenv("KC_DEBUG"))
			printf("%s(): xmlCopyNode() error!\n", __func__);

		return(0);
	}

	/* duplicate the destination key */
	key_tmp_dst = xmlCopyNode(*key_dst, 2);
	if (!key_tmp_dst) {
		dprintf(STDERR_FILENO, "ERROR: Error duplicating destination entry!\n");

		if (getenv("KC_DEBUG"))
			printf("%s(): xmlCopyNode() error!\n", __func__);

		return(0);
	}


	/* We need two key_tmp_* variables, because xmlReplaceNode() unlinks both its parameters */
	xmlReplaceNode(*key_src, key_tmp_dst);
	xmlFreeNode(*key_src);

	xmlReplaceNode(*key_dst, key_tmp_src);
	xmlFreeNode(*key_dst);

	*key_src = key_tmp_src;	/* the original "source" index */
	*key_dst = key_tmp_dst;	/* the original "destination" index */

	return(1);
} /* swap_keys() */
