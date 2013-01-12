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
#else
		e_line = readline("");
#endif
		if (!e_line) {
#ifndef _READLINE
			el_reset(e);

			/* re-enable the default prompt */
			if (el_set(e, EL_PROMPT, prompt_str) != 0) {
				perror("el_set(EL_PROMPT)");
			}
			/* re-enable history */
			if (el_set(e, EL_HIST, history, eh) != 0) {
				perror("el_set(EL_HIST)");
			}
#endif
			free(line); line = NULL;
			return;
		}

		if (strncmp(e_line, "yes", 3) != 0) {
			free(line); line = NULL;
			return;
		}
	}

	if (xmlSaveFormatFileEnc(export_filename, db, "UTF-8", XML_SAVE_FORMAT) > 0)
		puts("Export OK!");
	else
		printf("failed to export to '%s'.\n", export_filename);

	free(line); line = NULL;
} /* cmd_export() */
