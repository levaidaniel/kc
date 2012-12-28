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
#include <wctype.h>

#include "common.h"
#include "commands.h"


xmlChar *get_line(xmlChar *, int, int);
xmlChar *parse_newlines(xmlChar *, char);


extern char		batchmode;

#ifndef _READLINE
extern EditLine		*e;
#endif


void
cmd_getnum(int idx, size_t space)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*key = NULL, *value = NULL, *value_nl = NULL, *line = NULL, *line_randomed = NULL, *tmp = NULL;

	int		lines = 0, i = 0, value_len = 0;
	size_t		line_len = 0, line_randomed_len = 0, erase_len = 0;
	char		rc = 0;
	char		*rand_str = NULL;


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
			perror("el_set(EL_PROMPT)");
		}
		if (el_set(e, EL_UNBUFFERED, 1) != 0) {
			perror("el_set(EL_UNBUFFERED)");
			return;
		}
#else
		rl_prep_terminal(1);
#endif

		idx = 1;	/* from hereafter 'idx' will be our requested line number */
		while (rc != 'q') {	/* quit for 'q' */
			if (batchmode)
				puts("");

			printf("[%s] ", key);	/* print the key */

			/* if multiline, prefix the line with a line number */
			if (lines > 1)
				printf("[%d/%d] ", idx, lines);

			/* get a line out from the value */
			line = get_line(value_nl, value_len, idx);
			line_len = strlen((const char *)line);

			if (space) {	/* if random padding is requested */
				line_randomed_len = line_len + line_len * space + space + 1;
				line_randomed = calloc(1, line_randomed_len); malloc_check(line_randomed);

				/* begin with the random string */
				rand_str = get_random_str(space, 0);
				if (!rand_str)
					return;
				strlcat((char *)line_randomed, rand_str, line_randomed_len);
				free(rand_str); rand_str = NULL;
				for (i=0;i < (int)line_len;i++) {
					/* append a character from the line */
					tmp = xmlUTF8Strsub(line, i, 1);
					strlcat((char *)line_randomed, (const char *)tmp, line_randomed_len);
					xmlFree(tmp); tmp = NULL;

					/* append a random string */
					rand_str = get_random_str(space, 0);
					if (!rand_str)
						return;
					strlcat((char *)line_randomed, rand_str, line_randomed_len);
					free(rand_str); rand_str = NULL;
				}
				line_randomed[(long)(line_randomed_len - 1)] = '\0';

				printf("%s", line_randomed);
			} else {
				printf("%s", line);
			}
#ifdef _READLINE		
			rl_redisplay();		
#endif
			xmlFree(line); line = NULL;
			free(line_randomed); line_randomed = NULL;

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
				erase_len =	strlen((const char *)key) + 3 +					/* add the key + "[" + "]" + " " */
						(space ? line_len + line_len * space + space : line_len) +	/* add the random chars too */
						(lines > 1 ? digit_length(idx) + digit_length(lines) + 4 : 0);	/* add the line number prefix too + "[" + "/" + "]" + " " */
				for (i=0; i < (int)erase_len; i++)
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
					case 10:
						if (idx < lines)
							idx++;
						break;
					/* line backward */
					case 'b':
					case 'p':
					case 'k':
					case '-':
					case 8:
						if (idx - 1 > 0)
							idx--;
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
						if (idx - 1 > 0  &&  idx < lines) {
							idx = rc - 48;
						}
						break;
					default:
						break;
				}
			}
		}

		xmlFree(key); key = NULL;
		xmlFree(value_nl); value_nl = NULL;

#ifndef _READLINE
		/* re-enable the default prompt */
		if (el_set(e, EL_PROMPT, prompt_str) != 0) {
			perror("el_set(EL_PROMPT)");
		}
		el_set(e, EL_UNBUFFERED, 0);
#else
		rl_deprep_terminal();
#endif
	} else
		puts("invalid index!");
} /* cmd_getnum() */


/*
 * Get the idx'th line from the value.
 */
xmlChar *
get_line(xmlChar *value_nl, int value_len, int idx)
{
	xmlChar	*line = NULL;

	int	nl = 1, pos = 0, tmp = 0;
	size_t	line_len = 0;


	/* find out the start position (pos) of the requested line number (idx) */
	for (pos = 0;(pos < value_len)  &&  (idx > nl); pos++) {
		if (value_nl[pos] == '\n') {	/* we've found a newline */
			nl++;
		}
	}

	tmp = pos;
	/* count the requested line length */
	while (value_nl[pos] != '\n'  &&  value_nl[pos] != '\0') {
		line_len++;
		pos++;
	}

	pos = tmp;
	tmp = 0;
	line = malloc(line_len + 1); malloc_check(line);
	/* copy out the requested line */
	while (value_nl[pos] != '\n'  &&  value_nl[pos] != '\0'  &&  tmp < (int)line_len) {
		line[tmp++] = value_nl[pos++];
	}
	line[(long)line_len] = '\0';

	return(line);
} /* get_line() */


xmlChar *
parse_newlines(xmlChar *line, char dir)		/* dir(direction): "\n" -> '\n' = 0, '\n' -> "\n" = 1 */
{
	xmlChar		*ret = NULL;
	int		i = 0, j = 0;
	size_t		nlnum = 0, ret_len = 0;


	if (!line)
		return(xmlStrdup(BAD_CAST ""));


	if (dir) {
		/*
		 * count the number of '\n' characters in the string, and use it later to figure how many bytes
		 * will be the new string, with replaced newline characters.
		 */
		for (i=0; i < (int)xmlStrlen(line); i++)
			if (line[i] == '\n')	/* we got a winner... */
				nlnum++;

		ret_len = xmlStrlen(line) + nlnum + 1;
	} else {
		/*
		 * count the number of "\n" sequences in the string, and use it later to figure how many bytes
		 * will be the new string, with replaced newline sequences.
		 */
		for (i=0; i < (int)xmlStrlen(line); i++) {
			if (line[i] == '\\') {	/* got an escape character, we better examine it... */
				if (line[i+1] == '\\')	/* the "\\n" case. the newline is escaped, so honor it */
					i += 2;		/* skip these. don't count them, because they are not newlines */
				else if (line[i+1] == 'n')	/* we got a winner... */
					nlnum++;
			}
		}

		ret_len = xmlStrlen(line) - nlnum + 1;
	}
	ret = malloc(ret_len); malloc_check(ret);


	if (dir) {
		/* replace the real newline characters with newline sequences ("\n"); */
		for (i=0; i < (int)xmlStrlen(line); i++) {
			if (line[i] == '\n') {			/* we got a winner... */
				ret[j++] = '\\';		/* replace with NL character */
				ret[j++] = 'n';			/* replace with NL character */
			} else
				ret[j++] = line[i];			/* anything else will just go into the new string */
		}
	} else {
		/* replace the newline sequences with real newline characters ('\n'); */
		for (i=0; i < (int)xmlStrlen(line); i++) {
			if (line[i] == '\\') {	/* got an escape character, we better examine it... */
				if (line[i+1] == '\\') {	/* the "\\n" case. the newline is escaped, so honor it */
					ret[j++] = line[i];	/* copy it as if nothing had happened */
					ret[j++] = line[++i];
				} else if (line[i+1] == 'n') {	/* we got a winner... */
					ret[j++] = '\n';	/* replace with NL character */
					i++;			/* skip the 'n' char from "\n" */
				} else
					ret[j++] = line[i];	/* anything else will just go into the new string */
			} else
				ret[j++] = line[i];		/* anything else will just go into the new string */
		}
	}

	ret[(long)(ret_len - 1)] = '\0';		/* close that new string safe and secure. */


	return(ret);	/* return the result; we've worked hard on it. */
} /* parse_newlines() */
