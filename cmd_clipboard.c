/*
 * Copyright (c) 2011-2014 LEVAI Daniel
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


extern db_parameters	db_params;
extern xmlNodePtr	keychain;
extern BIO		*bio_chain;


void
cmd_clipboard(const char *e_line, command *commands)
{
	xmlNodePtr		db_node = NULL;
	xmlChar			*key = NULL, *value = NULL, *value_nl = NULL, *value_line = NULL;
	char			*cmd_line = NULL, *cmd = NULL, *inv = NULL;
	unsigned long int	idx = 0, line_req = 1, lines = 0, i = 0;
	long int		value_line_len = 0, value_len = 0;
	unsigned char		app = 0;	/* 1=tmux, 2=xclip PRIMARY, 3=xclip CLIPBOARD */

	char		**fork_argv = NULL;
	int		child;
	int		pipefd[2];


	cmd_line = strdup(e_line); malloc_check(cmd_line);

	cmd = strtok(cmd_line, " ");
	if (!cmd) {
		puts(commands->usage);

		free(cmd_line); cmd_line = NULL;
		return;
	}

	if (strcmp(cmd, "tmux") == 0)
		app = 1;
	else if (strcmp(cmd, "xclip") == 0)
		app = 2;
	else if (strcmp(cmd, "Xclip") == 0)
		app = 3;

	if (app == 0) {
		puts(commands->usage);

		free(cmd_line); cmd_line = NULL;
		return;
	}


	cmd = strtok(NULL, " ");	/* first parameter, the index number */
	if (!cmd) {
		puts(commands->usage);

		free(cmd_line); cmd_line = NULL;
		return;
	}

	errno = 0;
	idx = strtoul((const char *)cmd, &inv, 10);
	if (inv[0] != '\0'  ||  errno != 0  ||  cmd[0] == '-') {
		puts(commands->usage);

		free(cmd_line); cmd_line = NULL;
		return;
	}


	cmd = strtok(NULL, " ");	/* second, optional parameter, the requested line number */
	if (cmd) {
		errno = 0;
		line_req = strtoul((const char *)cmd, &inv, 10);
		if (inv[0] != '\0'  ||  errno != 0  ||  cmd[0] == '-') {
			puts(commands->usage);

			free(cmd_line); cmd_line = NULL;
			return;
		}
		if (line_req < 1) {
			puts(commands->usage);

			free(cmd_line); cmd_line = NULL;
			return;
		}
	}

	free(cmd_line); cmd_line = NULL;


	db_node = find_key(idx);
	if (db_node) {
		value = xmlGetProp(db_node, BAD_CAST "value");
		key = xmlGetProp(db_node, BAD_CAST "name");
		value_nl = parse_newlines(value, 0);
		xmlFree(value); value = NULL;

		/* count how many (new)lines are in the string */
		value_len = xmlStrlen(value_nl);
		for (i=0; i < value_len; i++)
			if (value_nl[i] == '\n')
				lines++;
		lines++;

		/* If the requested line number is greater than the
		 * maximum, use the maximum.
		 */
		if (line_req > lines)
			line_req = lines;

		/* get a line out from the value */
		value_line = get_line(value_nl, line_req);
		value_line_len = xmlStrlen(value_line);

		/* This is duplicated in cmd_getnum.c */
		switch (app) {
			case 1:
				/* Copy value to tmux paste buffer */
				switch (child = fork()) {
					case -1:
						perror("\nCouldn't fork(2) for tmux(1)");
						break;
					case 0:	/* Child */
						close(0);
						close(1);

						if (bio_chain)
							BIO_free_all(bio_chain);

						if (db_params.db_file) {
							if (close(db_params.db_file) == -1) {
								perror("child: close(database file)");
								exit(EXIT_FAILURE);
							}
						}

						fork_argv = malloc(5 * sizeof(char *)); malloc_check(fork_argv);
						fork_argv[0] = "tmux";
						fork_argv[1] = "set-buffer";
						fork_argv[2] = "--";
						fork_argv[3] = (char *)value_line;
						fork_argv[4] = NULL;

						if (execvp(fork_argv[0], fork_argv) == -1)
							fprintf(stderr, "tmux: %s", strerror(errno));

						quit(EXIT_FAILURE);

						break;
					default: /* Parent */
						printf("Copying '%s' to tmux paste buffer.\n", key);
						break;
				}

				break;
			case 2:
			case 3:
				/* Copy value to X11 clipboard, using xclip(1) */

				pipe(pipefd);

				switch (child = fork()) {
					case -1:
						perror("\nCouldn't fork(2) for xclip(1)");
						break;
					case 0:	/* Child */
						close(0);
						close(1);
						close(pipefd[1]);

						if (bio_chain)
							BIO_free_all(bio_chain);

						if (db_params.db_file) {
							if (close(db_params.db_file) == -1) {
								perror("child: close(database file)");
								exit(EXIT_FAILURE);
							}
						}

						fork_argv = malloc(4 * sizeof(char *)); malloc_check(fork_argv);
						fork_argv[0] = "xclip";
						fork_argv[1] = "-selection";
						if (app == 2) {
							fork_argv[2] = "primary";
						} else if (app == 3) {
							fork_argv[2] = "clipboard";
						}
						fork_argv[3] = NULL;

						/* stdin becomes the read end of the pipe in the child,
						 * and the exec'd process will have the same environment. */
						dup2(pipefd[0], 0);
						if (execvp(fork_argv[0], fork_argv) == -1)
							fprintf(stderr, "xclip: %s", strerror(errno));

						quit(EXIT_FAILURE);

						break;
					default: /* Parent */
						/* Write the value to the pipe's write end, which will
						 * appear in the child's stdin (pipe's read end). */
						close(pipefd[0]);
						write(pipefd[1], value_line, value_line_len);
						close(pipefd[1]);

						printf("Copying '%s' to X11 clipboard.\n", key);

						break;
				}

				break;
		}
	} else
		puts("Invalid index!");


	xmlFree(key); key = NULL;
	xmlFree(value_line); value_line = NULL;
	xmlFree(value_nl); value_nl = NULL;
} /* cmd_clipboard() */
