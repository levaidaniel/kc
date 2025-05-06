/*
 * Copyright (c) 2011-2025 LEVAI Daniel
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


/* 'erase_len'
 * add the key + "[" + "]" + " "
 * add the random chars too
 * add the line number prefix too + "[" + "/" + "]" + " "
 */
#define		ERASE_LEN \
	strlen((const char *)key) + 3 + \
	(spice > 0 ? line_len + line_len * spice + spice : line_len) + \
	(lines > 1 ? digit_length(idx) + digit_length(lines) + 4 : 0)


unsigned long int	digit_length(unsigned long int);


extern db_parameters	db_params;
extern char		batchmode;

#ifndef _READLINE
extern EditLine		*e;
#endif

extern BIO		*bio_chain;


void
cmd_getnum(const unsigned long int idx, const unsigned long int spice)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*key = NULL, *value = NULL, *value_nl = NULL, *line = NULL, *line_randomed = NULL, *tmp = NULL;

	unsigned long int	lines = 0, line_req = 1, value_len = 0, line_len = 0, line_randomed_len = 0, i = 0;
	char		rc = 0;
	char		*rand_str = NULL;
	char		**fork_argv = NULL;
	int		pipefd[2];


	db_node = find_key(idx);
	if (db_node) {
		key = xmlGetProp(db_node, BAD_CAST "name");
		value = xmlGetProp(db_node, BAD_CAST "value");
		value_nl = parse_newlines(value, 0);
		xmlFree(value); value = NULL;

		value_len = xmlStrlen(value_nl);

		/* count how many (new)lines are in the string */
		for (i=0; i < value_len; i++)
			if (value_nl[i] == '\n')
				lines++;
		lines++;


#ifndef _READLINE
		/* clear the prompt temporarily */
		if (el_set(e, EL_PROMPT, el_prompt_null) != 0) {
			perror("ERROR: el_set(EL_PROMPT)");
		}
		if (el_set(e, EL_UNBUFFERED, 1) != 0) {
			perror("ERROR: el_set(EL_UNBUFFERED)");

			xmlFree(key); key = NULL;
			xmlFree(value_nl); value_nl = NULL;
			return;
		}
#else
		rl_prep_terminal(1);
#endif

		while (rc != 'q'  &&  rc != 4) {	/* quit for 'q' or EOT */
			if (batchmode)
				puts("");

			printf("[%s] ", key);	/* print the key */

			/* if multiline, prefix the line with a line number */
			if (lines > 1)
				printf("[%lu/%lu] ", line_req, lines);

			/* get a line out from the value */
			line = get_line(value_nl, line_req);
			line_len = xmlStrlen(line);

			if (spice > 0) {	/* if random padding is requested */
				line_randomed_len = line_len + line_len * spice + spice + 1;
				line_randomed = calloc(1, line_randomed_len); malloc_check(line_randomed);

				/* begin with the random string */
				rand_str = get_random_str(spice, 1);
				if (!rand_str) {
					xmlFree(key); key = NULL;
					xmlFree(value_nl); value_nl = NULL;
					xmlFree(line); line = NULL;
					free(line_randomed); line_randomed = NULL;

					return;
				}
				(void) strlcat((char *)line_randomed, rand_str, line_randomed_len);
				free(rand_str); rand_str = NULL;
				for (i = 0; i < line_len; i++) {
					/* append a character from the line */
					tmp = xmlUTF8Strsub(line, i, 1);
					(void) strlcat((char *)line_randomed, (const char *)tmp, line_randomed_len);
					xmlFree(tmp); tmp = NULL;

					/* append a random string */
					rand_str = get_random_str(spice, 1);
					if (!rand_str) {
						xmlFree(key); key = NULL;
						xmlFree(value_nl); value_nl = NULL;
						xmlFree(line); line = NULL;
						free(line_randomed); line_randomed = NULL;

						return;
					}
					(void) strlcat((char *)line_randomed, rand_str, line_randomed_len);
					free(rand_str); rand_str = NULL;
				}
				line_randomed[line_randomed_len - 1] = '\0';

				xmlFree(line); line = NULL;
				line = line_randomed;
			}

			printf("%s", line);
#ifdef _READLINE
			rl_redisplay();
#endif

			if (batchmode) {
				rc = 'q';
				puts("");
			} else {
				/* this is the prompt, after displaying the value */
#ifndef _READLINE
				el_getc(e, &rc);
#else
				rc = rl_read_key();
#endif

				/* erase (overwrite) the previously written value with spaces */
				printf("\r");
				for (i = 0; i < ERASE_LEN; i++)
					putchar(' ');

				printf("\r");


				/* process the keypress */
				switch (rc) {
					/* line forward */
					case 'f':
					case 'n':
					case 'j':
					case '+':
					case ' ':
					case '>':
					case ']':
					case '}':
					case 10:	/* editline */
					case 13:	/* readline */
						if (line_req < lines)
							line_req++;
						break;
					/* line backward */
					case 'b':
					case 'p':
					case 'k':
					case '-':
					case '<':
					case '[':
					case '{':
					case 8:
						if (line_req - 1 > 0)
							line_req--;
						break;
					/* jump to the requested line */
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						if ((unsigned char)(rc - 48) <= lines)
							line_req = rc - 48;
						break;
					case 't':
						/* This is duplicated in cmd_clipboard.c */
						/* Copy value to tmux paste buffer */

						switch (fork()) {
							case -1:
								perror("\nERROR: Couldn't fork(2) for tmux(1)");
								break;
							case 0:	/* Child */
								close(0);	/* This is also needed for Editline's UNBUFFERED mode to continue to work properly. */
								close(1);

								if (bio_chain)
									BIO_free_all(bio_chain);

								if (db_params.db_file) {
									if (close(db_params.db_file) == -1) {
										perror("ERROR: child: close(database file)");
										exit(EXIT_FAILURE);
									} else {
										db_params.db_file = -1;
									}
								}

								fork_argv = malloc(5 * sizeof(char *)); malloc_check(fork_argv);
								fork_argv[0] = "tmux";
								fork_argv[1] = "set-buffer";
								fork_argv[2] = "--";
								fork_argv[3] = (char *) line;
								fork_argv[4] = NULL;

								if (execvp(fork_argv[0], fork_argv) == -1)
									dprintf(STDERR_FILENO, "ERROR: tmux: %s\n", strerror(errno));

								quit(EXIT_FAILURE);

								break;
							default: /* Parent */
								printf("Copying value to tmux clipboard.\n");
#ifdef _READLINE
								rl_redisplay();
#endif
								break;
						}

						break;
					case 'x':
					case 'X':
						/* This is duplicated in cmd_clipboard.c */
						/* Copy value to X11 clipboard, using xclip(1) */

						pipe(pipefd);

						switch (fork()) {
							case -1:
								perror("\nERROR: Couldn't fork(2) for xclip(1)");
								break;
							case 0:	/* Child */
								close(0);	/* This is also needed for Editline's UNBUFFERED mode to continue to work properly. */
								close(1);
								close(pipefd[1]);

								if (bio_chain)
									BIO_free_all(bio_chain);

								if (db_params.db_file) {
									if (close(db_params.db_file) == -1) {
										perror("ERROR: child: close(database file)");
										exit(EXIT_FAILURE);
									} else {
										db_params.db_file = -1;
									}
								}

								fork_argv = malloc(4 * sizeof(char *)); malloc_check(fork_argv);
								fork_argv[0] = "xclip";
								fork_argv[1] = "-selection";
								if (rc == 'x') {
									fork_argv[2] = "primary";
								} else if (rc == 'X') {
									fork_argv[2] = "clipboard";
								}
								fork_argv[3] = NULL;

								/* stdin becomes the read end of the pipe in the child,
								 * and the exec'd process will have the same environment. */
								dup2(pipefd[0], 0);
								if (execvp(fork_argv[0], fork_argv) == -1)
									dprintf(STDERR_FILENO, "ERROR: xclip: %s\n", strerror(errno));

								quit(EXIT_FAILURE);

								break;
							default: /* Parent */
								/* Write the value to the pipe's write end, which will
								 * appear in the child's stdin (pipe's read end). */
								close(pipefd[0]);
								write(pipefd[1], line, line_len);
								close(pipefd[1]);

								printf("Copying value to X11 clipboard.\n");
#ifdef _READLINE
								rl_redisplay();
#endif
								break;
						}

						break;
					default:
						break;
				}
			}

			xmlFree(line); line = NULL;
		}

		xmlFree(key); key = NULL;
		xmlFree(value_nl); value_nl = NULL;

#ifndef _READLINE
		/* re-enable the default prompt */
		if (el_set(e, EL_PROMPT, prompt_str) != 0) {
			perror("ERROR: el_set(EL_PROMPT)");
		}
		el_set(e, EL_UNBUFFERED, 0);
#else
		rl_deprep_terminal();
#endif
	} else
		puts("Invalid index!");
} /* cmd_getnum() */


unsigned long int
digit_length(unsigned long int digit)
{
	unsigned long int	length = 1;


	while ((digit / 10) != 0) {
		digit /= 10;
		length++;
	}

	return(length);
} /* digit_length() */
