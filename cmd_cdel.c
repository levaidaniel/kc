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

#include "common.h"
#include "commands.h"


extern xmlNodePtr	keychain;
extern char		dirty;


void cmd_cdel(EditLine *e, ...) {

va_list		ap;

History 	*eh = NULL;

xmlNodePtr	db_node = NULL, db_node_tmp = NULL;
xmlChar		*cname_locale = NULL, *cname = NULL;

command		*commands = NULL;

const char	*e_line = NULL;
int		e_count = 0;
char		*line = NULL;


	va_start(ap, e);

	line = va_arg(ap, char *);
	line[strlen(line) - 1] = '\0';		// remove the newline character from the end

	eh = va_arg(ap, History *);
	va_arg(ap, BIO *);
	commands = va_arg(ap, command *);

	va_end(ap);

	strtok(line, " ");		// remove the command from the line
	cname_locale = BAD_CAST strtok(NULL, " ");	// assign the command's parameter
	if (!cname_locale) {
		puts(commands->usage);
		return;
	}

	cname = convert_utf8(cname_locale, 0);

	db_node = find_keychain(cname);
	if (db_node) {
		if (xmlUTF8Charcmp(xmlGetProp(keychain, BAD_CAST "name"),
				   xmlGetProp(db_node, BAD_CAST "name")) == 0) {	// don't allow to delete the actual keychain. this saves us trouble.

			puts("Can not delete the actual keychain!");
		} else {
			// disable history temporarily
			if (el_set(e, EL_HIST, history, NULL) != 0) {
				perror("el_set(EL_HIST)");
			}
			// clear the prompt temporarily
			if (el_set(e, EL_PROMPT, e_prompt_null) != 0) {
				perror("el_set(EL_PROMPT)");
			}


			free(cname);
			cname = xmlGetProp(db_node, BAD_CAST "name");
			cname_locale = convert_utf8(cname, 1);
			printf("Do you really want to delete '%s'? <yes/no> ", cname_locale);

			e_line = el_gets(e, &e_count);
			if (!e_line) {
				perror("input");
				return;
			}

			if (strncmp(e_line, "yes", 3) == 0) {
				// check if it was the last keychain,
				// by checking its neighbors if they exist.
				// we don't allow to delete the last keychain
				db_node_tmp = db_node->next;

				// first, search forward.
				while (db_node_tmp) {
					if (db_node_tmp->type == XML_ELEMENT_NODE) {	// we only care about ELEMENT nodes
						keychain = db_node_tmp;
						printf("actual keychain has changed to '%s' (next).\n", xmlGetProp(keychain, BAD_CAST "name"));
						break;
					}

					db_node_tmp = db_node_tmp->next;
				}

				// if we didn't find a consequent valid node,
				// search backwards.
				if (!db_node_tmp) {
					db_node_tmp = db_node->prev;

					while (db_node_tmp) {
						if (db_node_tmp->type == XML_ELEMENT_NODE) {	// we only care about ELEMENT nodes
							keychain = db_node_tmp;
							break;
						}

						db_node_tmp = db_node_tmp->prev;
					}
				}

				// now 'keychain' points to a sibling (next or previous one)

				if (db_node_tmp) {
					printf("'%s'", cname_locale);
					free(cname_locale);

					db_node_tmp = db_node->prev;
					xmlUnlinkNode(db_node_tmp);
					xmlFreeNode(db_node_tmp);

					xmlUnlinkNode(db_node);
					xmlFreeNode(db_node);

					puts(" deleted");
				} else
					puts("Can not delete the last keychain!\n");
			}


			// re-enable the default prompt
			if (el_set(e, EL_PROMPT, e_prompt) != 0) {
				perror("el_set(EL_PROMPT)");
			}
			// re-enable history
			if (el_set(e, EL_HIST, history, eh) != 0) {
				perror("el_set(EL_HIST)");
			}

			dirty = 1;
		}
	} else {
		printf("'%s' keychain not found.\n", cname_locale);
		free(cname);
	}
} /* cmd_cdel() */
