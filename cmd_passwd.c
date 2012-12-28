/*
 * Copyright (c) 2011, 2012 LEVAI Daniel
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

#ifndef _LINUX
#include <fcntl.h>
#include <readpassphrase.h>
#else
#include <sys/file.h>
#include <bsd/readpassphrase.h>
#endif


extern char		dirty;
extern size_t		pass_maxlen;


void
cmd_passwd(const char *e_line, command *commands)
{
	char		*pass = NULL;
	char		pass_prompt[15];
	char		*line = NULL;


	line = strdup(e_line);

	strtok(line, " ");			/* remove the command from the line */
	pass = strtok(NULL, " ");		/* assign the command's first parameter (password) */
	free(line); line = NULL;
	if (!pass) {				/* if we didn't get a password as a parameter */
		/* ask for the new password */
		strlcpy(pass_prompt, "New password: ", 15);
		pass = malloc(pass_maxlen + 1); malloc_check(pass);
		readpassphrase(pass_prompt, pass, pass_maxlen + 1, RPP_ECHO_OFF | RPP_REQUIRE_TTY);
	}

	dirty = 1;
} /* cmd_passwd() */
