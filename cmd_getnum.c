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


char get_line(xmlChar *, int, int *, char *, int);


extern xmlNodePtr	keychain;
extern char		batchmode;


void
cmd_getnum(EditLine *e, ...)
{
	va_list		ap;

	xmlNodePtr	db_node = NULL;
	xmlChar		*key_locale = NULL, *value_locale = NULL, *key = NULL, *value = NULL;

	int		idx = 0, newlines = 0, i = 0, pos = 0, space = 0, erase_len = 0, line_len = 0, value_len = 0;
	char		c = -1, rc = -1;
	char		*rand_str = NULL;


	va_start(ap, e);

	idx = va_arg(ap, int);
	space = va_arg(ap, int);

	va_end(ap);

	db_node = find_key(idx);
	if (db_node) {
		key = xmlNodeGetContent(db_node->children);
		key_locale = convert_utf8(key, 1);
		xmlFree(key); key = NULL;

		printf("[%s]\n", key_locale);	// print the key

		free(key_locale); key_locale = NULL;

		// clear the prompt temporarily
		if (el_set(e, EL_PROMPT, e_prompt_null) != 0) {
			perror("el_set(EL_PROMPT)");
		}
		if (el_set(e, EL_UNBUFFERED, 1) != 0) {
			perror("el_set(EL_UNBUFFERED)");
			return;
		}

		idx = 1;
		value = xmlNodeGetContent(db_node->children->next->children);
		value_locale = convert_utf8(value, 1);
		xmlFree(value); value = NULL;

		value_len = xmlStrlen(value_locale);

		// count how many lines are in the string
		for (i=0; i < value_len; i++)
			if (value_locale[i] == '\n')
				newlines++;

		while (c != '\0') {		// handle multiline values

			if (newlines)
				printf("[%d/%d] ", idx, newlines + 1);	// if multiline, prefix the line with a line number

			if (space) {
				// print the first random character(s)
				rand_str = get_random_str(space, 0);
				if (!rand_str)
					return;
				printf("%s", rand_str);
				free(rand_str); rand_str = NULL;
			}

			do {
				c = value_locale[pos++];
				if (c == '\n'  ||  c == '\0')	// don't print the newlines and the NUL
					break;
				else {
					printf("%c", c);

					if (space) {
						// print random character(s)
						rand_str = get_random_str(space, 0);
						if (!rand_str)
							return;
						printf("%s", rand_str);
						free(rand_str); rand_str = NULL;
					}
				}
				line_len++;	// this is the actual line length
			} while (c != '\n'  &&  c != '\0');

			if (!batchmode) {
				// after printing a line, wait for user input
				rc = 0;
				while (	rc != 'q' &&
					rc != 10  &&  rc != 'f'  &&  rc != 'n'  &&  rc != 'j'  &&  rc != ' ' &&	// newline or 'f' ... display next line
					rc != 8   &&  rc != 'b'  &&  rc != 'p'  &&  rc != 'k'  &&		// backspace or 'b' ... display previous line
					(rc < '1'  ||  rc > '9')
					)
					el_getc(e, &rc);

				// erase (overwrite) the written value with spaces
				erase_len = (space ? line_len + line_len * space + space : line_len) +			// add the random characters too
						(newlines ? digit_length(idx) + digit_length(newlines + 1) + 4 : 0);	// add the line number prefix too
				printf("\r");
				for (i=0; i < erase_len; i++)
					putchar(' ');

				printf("\r");

				switch (rc) {
					// forward
					case 10:
					case 'f':
					case 'n':
					case 'j':
					case ' ':
						idx++;
					break;
					// backward
					case 8:
					case 'b':
					case 'p':
					case 'k':
						if (idx - 1 > 0) {	// don't go back, if we are already on the first line
							idx--;		// 'idx' is the line number we want!

							pos = get_line(value_locale, value_len, &pos, &c, idx);
						} else
							pos -= line_len + 1;	// rewind back to the current line's start, to display it again
					break;
					// quit
					case 'q':
						c = '\0';	// this is our exit condition
					break;
					// we got a number (this will be a line number)
					default:
						rc -= 48;		// 'idx' is the line number we want and 'rc' is the ascii version of the line number we got
						if ( rc <= newlines + 1 ) {
							idx = rc;
							pos = get_line(value_locale, value_len, &pos, &c, idx);
						} else {
							pos -= line_len + 1;	// rewind back to the current line's start, to display it again
							c = value_locale[pos];
						}
					break;
				}
			} else {
				// if we are in batch mode
				idx++;
				puts("");
			}

			line_len = 0;	// this is the actual line length
		}

		free(value_locale); value_locale = NULL;

		// re-enable the default prompt
		if (el_set(e, EL_PROMPT, e_prompt) != 0) {
			perror("el_set(EL_PROMPT)");
		}
		el_set(e, EL_UNBUFFERED, 0);
	} else {
		puts("invalid index!");
	}
} /* cmd_getnum() */


char
get_line(xmlChar *value_locale, int value_len, int *pos, char *c, int idx)
{
	int	i = 1;	// this counts how many '\n' we've found so far

	// if we want the first line (which can't be identified like the rest
	// of the lines: by presuming a '\n' character before the line)
	// just set everything to the beginning, and break; from this switch()
	if (idx == 1) {
		*pos = 0;
		*c = value_locale[*pos];
		return(*pos);
	}

	for(*pos=0; *pos < value_len; (*pos)++) {	// search the entire string
		if (i < idx) {			// while newline count ('i') is smaller than the line number requested
			if (value_locale[*pos] == '\n')	// we found a '\n'
				i++;
		} else
			break;
	}

	*c = value_locale[*pos];	// set the current character ('c') to the position's character in 'value_locale'

	return(*pos);
} /* get_line() */
