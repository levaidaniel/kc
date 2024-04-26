/*
 * Copyright (c) 2011-2024 LEVAI Daniel
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


void
cmd_clear(const char *e_line, command *commands)
{
	int	count = 0, i = 0, j = 0;
	char	*cmd = NULL, *line = NULL, *inv = NULL;


	line = strdup(e_line); malloc_check(line);


	cmd = strtok(line, " ");
	if (!cmd) {
		puts(commands->usage);

		free(line); line = NULL;
		return;
	}


	cmd = strtok(NULL, " ");	/* first optional parameter, the multiplier */
	if (cmd) {
		errno = 0;
		count = strtoul((const char *)cmd, &inv, 10);
		if (inv[0] != '\0'  ||  errno != 0  ||  cmd[0] == '-') {
			puts(commands->usage);

			free(line); line = NULL;
			return;
		}
	} else
		count = 1;


	free(line); line = NULL;


	if (count == 0)
		count++;

	if (count > 10)
		count = 10;


	for (i=0; i<count; i++)
		for (j=0; j<100; j++)
			puts("");
} /* cmd_clear() */
