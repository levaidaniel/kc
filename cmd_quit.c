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


extern xmlDocPtr	db;
extern char		dirty;
extern char		batchmode;
extern BIO		*bio_chain;

#ifndef _READLINE
extern EditLine		*e;
extern History		*eh;
extern HistEvent	eh_ev;
#endif


void
cmd_quit(const char *e_line, command *commands)
{
#ifndef _READLINE
	int		e_count = 0;
#endif


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

		printf("Do you want to write the changes? <yes/no> ");
#endif

#ifndef _READLINE
		e_line = el_gets(e, &e_count);
#else
		e_line = readline("Do you want to write the changes? <yes/no> ");
#endif
		if (!e_line) {
			perror("input");
#ifndef _READLINE
			el_reset(e);
#endif
			return;
		}

		if (strncmp(e_line, "yes", 3) == 0)
			cmd_write(e_line, commands);
		else
			puts("Changes were NOT saved.");
	}

	quit(EXIT_SUCCESS);
} /* cmd_quit() */
