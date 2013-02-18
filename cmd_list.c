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


extern xmlDocPtr	db;
extern xmlNodePtr	keychain;

#ifndef _READLINE
extern EditLine		*e;
#endif

extern char		batchmode;


void
cmd_list(const char *e_line, command *commands)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*key = NULL;

	int		idx = 0, pager = 0, pager_show = 0;

	char		*line = NULL;
	char		rc = 0;


	if (getenv("KC_DEBUG")) {
		xmlSaveFormatFileEnc("-", db, "UTF-8", XML_SAVE_FORMAT);
		printf("#BEGIN\n");
	}

	line = strdup(e_line);

	if (sscanf(e_line, "%*s %d", &pager) <= 0)
		pager = 20;

	if (pager > 100  ||  pager < 0)
		pager = 20;

	pager_show = pager;


	if (!batchmode) {
#ifndef _READLINE
		/* clear the prompt temporarily */
		if (el_set(e, EL_PROMPT, el_prompt_null) != 0) {
			perror("el_set(EL_PROMPT)");
		}
		if (el_set(e, EL_UNBUFFERED, 1) != 0) {
			perror("el_set(EL_UNBUFFERED)");
			free(line); line = NULL;
			return;
		}
#else
		rl_prep_terminal(1);
#endif
	}


	db_node = keychain->children;
	while (db_node) {
		if (db_node->type == XML_ELEMENT_NODE) {	/* we only care about ELEMENT nodes */
			/* Pager */
			if (	(!batchmode  &&
				rc != 'Q'  &&  idx == pager_show  &&  pager != 0)
				||  (rc == 13  ||  rc == 10)) {

				/* Brief pager usage info. */
				printf("[SPC,RET,0-9,EOT,q,Q,?]");
				fflush(stdout);

				rc = 0;
				while (	rc != ' '  &&  rc != 13  &&  rc != 10  &&
					rc != 4  &&  rc != 'q'  &&
					(rc < '0'  ||  rc > '9')  &&
					rc != 'Q') {
#ifndef _READLINE
					el_getc(e, &rc);
#else
					rc = rl_read_key();
#endif
					/* Full pager usage info. */
					if (rc == '?')
						puts("\n <SPACE>:\tNext page\n <ENTER>:\tNext line\n 1-9:\t\tNew pager value\n 'q', <EOT>:\tStop\n 'Q', 0:\tDisplay all\n");
				}

				/* Delete brief pager usage info. */
				printf("\r                       \r");

				if (rc == 4  ||  rc == 'q')
					break;

				/* User has modified the pager's value by pressing a number > 0 */
				if (rc >= '0'  &&  rc <= '9')
					pager = rc - 48;

				pager_show = idx + pager;
			}

			key = xmlGetProp(db_node, BAD_CAST "name");
			printf("%d. %s\n", idx++, key);
			xmlFree(key); key = NULL;
		}

		db_node = db_node->next;
	}

	if (!batchmode) {
#ifndef _READLINE
		/* re-enable the default prompt */
		if (el_set(e, EL_PROMPT, prompt_str) != 0) {
			perror("el_set(EL_PROMPT)");
		}
		el_set(e, EL_UNBUFFERED, 0);
#else
		rl_deprep_terminal();
#endif
	}

	if (idx == 0)
		puts("Empty keychain.");

	free(line); line = NULL;

	if (getenv("KC_DEBUG"))
		printf("#END\n");
} /* cmd_list() */
