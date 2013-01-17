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


#include <sys/stat.h>

#include "common.h"
#include "commands.h"


#ifndef _READLINE
extern EditLine		*e;
extern History		*eh;
#endif

extern xmlDocPtr	db;


void
cmd_export(const char *e_line, command *commands)
{
	xmlDocPtr	db_tmp = NULL;
	xmlNodePtr	keychain = NULL, keychain_tmp = NULL, root_node_tmp = NULL;
	xmlChar		*cname = NULL;

	char		*export_filename = NULL, *line = NULL;
	struct stat	st;

#ifndef _READLINE
	int		e_count = 0;
#endif


	line = strdup(e_line);

	strtok(line, " ");			/* remove the command from the line */
	export_filename = strtok(NULL, " ");	/* assign the command's parameter */
	if (!export_filename) {
		puts(commands->usage);
		free(line); line = NULL;
		return;
	}

	cname = BAD_CAST strtok(NULL, " ");	/* assign the command's parameter */
	if (cname) {
		/* A 'keychain' was specified, so export only that one.
		 * We must create a new xmlDoc and copy the specified keychain to it.
		 */

		keychain = find_keychain(cname);
		if (!keychain) {
			printf("'%s' keychain not found.\n", cname);
			free(line); line = NULL;
			return;
		}

		/* create the new document */
		db_tmp = xmlNewDoc(BAD_CAST "1.0");
		if (!db_tmp) {
			puts("Could not create the new XML document for export!");
			free(line); line = NULL;
			return;
		}

		xmlCreateIntSubset(db_tmp, BAD_CAST "kc", NULL, BAD_CAST "kc.dtd");

		/* A new root node */
		root_node_tmp = xmlNewNode(NULL, BAD_CAST "kc");
		if (!root_node_tmp) {
			puts("Could not create the new root node for export!");
			xmlFreeDoc(db_tmp);
			free(line); line = NULL;
			return;
		}
		xmlDocSetRootElement(db_tmp, root_node_tmp);

		/* add the specified keychain's node to the export xmlDoc */
		xmlAddChild(root_node_tmp, xmlNewText(BAD_CAST "\n\t"));
		keychain_tmp = xmlCopyNode(keychain, 1);
		if (!keychain_tmp) {
			puts("Could not duplicate keychain for export!");
			xmlFreeDoc(db_tmp);
			free(line); line = NULL;
			return;
		}
		xmlAddChild(root_node_tmp, keychain_tmp);

		xmlAddChild(root_node_tmp, xmlNewText(BAD_CAST "\n"));
	} else
		/* save the whole document */
		db_tmp = db;

	if(lstat(export_filename, &st) == 0) {	/* if export filename exists */
#ifndef _READLINE
		/* disable history temporarily */
		if (el_set(e, EL_HIST, history, NULL) != 0) {
			perror("el_set(EL_HIST)");
		}
		/* clear the prompt temporarily */
		if (el_set(e, EL_PROMPT, el_prompt_null) != 0) {
			perror("el_set(EL_PROMPT)");
		}
#endif
		printf("Do you want to overwrite '%s'? <yes/no> ", export_filename);

#ifdef _READLINE
		rl_redisplay();
#endif

#ifndef _READLINE
		e_line = el_gets(e, &e_count);

		/* re-enable the default prompt */
		if (el_set(e, EL_PROMPT, prompt_str) != 0) {
			perror("el_set(EL_PROMPT)");
		}
		/* re-enable history */
		if (el_set(e, EL_HIST, history, eh) != 0) {
			perror("el_set(EL_HIST)");
		}
#else
		e_line = readline("");
#endif
		if (!e_line) {
#ifndef _READLINE
			el_reset(e);
#endif
			free(line); line = NULL;
			return;
		}

		if (strncmp(e_line, "yes", 3) != 0) {
			free(line); line = NULL;
			return;
		}
	}

	if (xmlSaveFormatFileEnc(export_filename, db_tmp, "UTF-8", XML_SAVE_FORMAT) > 0) {
		if (chmod(export_filename, S_IRUSR | S_IWUSR) < 0)
			puts("Couldn't change permissions of export file!");

		puts("Export OK!");
	} else
		printf("Failed to export to '%s'.\n", export_filename);

	if (cname)
		xmlFreeDoc(db_tmp);	/* if we saved a specific keychain, clean up the temporary xmlDoc and its tree. */

	free(line); line = NULL;
} /* cmd_export() */
