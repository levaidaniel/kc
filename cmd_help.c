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


extern command	*commands_first;


void
cmd_help(const char *e_line, command *commands)
{
	char		*got_command = NULL;

	char		*line = NULL;


	commands = commands_first;

	line = strdup(e_line);

	strtok(line, " ");			/* remove the command from the line */
	got_command = strtok(NULL, " ");	/* assign the command's parameter */

	if (got_command) {
		while (commands) {
			if (strcmp(commands->name, got_command) == 0) {
				printf("%s\n\n", commands->usage);
				printf("%s\n", commands->help);
				break;
			}
			commands = commands->next;
		}
		if (!commands)
			printf("Unknown command: '%s'\n", got_command);
	} else {
		puts("\nCommands:\n"
			"[Name]     -    [Usage]\n");

		while (commands) {
			printf("%-10s - \t%s\n", commands->name, commands->usage);

			commands = commands->next;
		}
		printf("\n%-10s - \t<number> [space]\n", "<number>");
		puts("\nEntering only a number in the command line will display the entry with the given index. You quit from the display with 'q' or EOT (usually CTRL+d).");
		puts("\nBy specifying another number (space) after the index, that many random characters will be displayed between the value's characters.");
		puts("\nYou navigate up/down through a multiline value's lines with keys j/k, n/p, f/b, +/-, [/], {/}, </>, <SPACE>, <ENTER>, <BACKSPACE>. Typing a number between 1-9 will jump directly to that line.");
		puts("\nFor a command's description, use 'help <command name>'.");
	}

	free(line); line = NULL;
} /* cmd_help() */
