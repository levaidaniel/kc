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


extern char		dirty;
extern char		batchmode;

#ifndef _READLINE
extern EditLine		*e;
extern History		*eh;
#endif


void
cmd_quit(const char *e_line, command *commands)
{
#ifndef _READLINE
	int		e_count = 0;
#endif
	char		*line = NULL;


	if (dirty  &&  !batchmode) {
#ifndef _READLINE
		/* disable history */
		if (el_set(e, EL_HIST, history, NULL) != 0) {
			perror("el_set(EL_HIST)");
		}
		/* clear the prompt */
		if (el_set(e, EL_PROMPT, el_prompt_null) != 0) {
			perror("el_set(EL_PROMPT)");
		}
#endif

		do {
#ifndef _READLINE
			printf("Do you want to write the changes? <yes/no> ");
			e_line = el_gets(e, &e_count);
#else
			e_line = readline("Do you want to write the changes? <yes/no> ");
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
				return;
			}

			line = strdup(e_line);

#ifndef _READLINE
			line[(long)(strlen(line) - 1)] = '\0';	/* remove the newline */
#endif

		} while (	strcasecmp(line, "yes") != 0  &&
				strcasecmp(line, "y") != 0  &&
				strcasecmp(line, "no") != 0  &&
				strcasecmp(line, "n") != 0);

		if (	strcasecmp(line, "yes") == 0  ||
			strcasecmp(line, "y") == 0)

			cmd_write(NULL, NULL);
		else
			puts("Changes were NOT saved.");
	}

	quit(EXIT_SUCCESS);
} /* cmd_quit() */
