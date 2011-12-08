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


#include <fcntl.h>

#include "common.h"


extern xmlNodePtr	keychain;

#ifdef _READLINE
xmlChar			*_rl_helper_var = NULL;
#endif


xmlNodePtr
find_keychain(xmlChar *cname_find)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*cname = NULL;

	char		*inv = NULL;
	int		i = 0;
	long		idx = -1;


	/* if we got a number */
	idx = strtol((const char *)cname_find, &inv, 10);
	if (strncmp(inv, "\0", 1) != 0) {
		idx = -1;
	}


	db_node = keychain->parent->children;

	while (db_node) {
		if (db_node->type != XML_ELEMENT_NODE) {	/* we only care about ELEMENT nodes */
			db_node = db_node->next;
			continue;
		}

		if (idx >= 0) {		/* if an index number was given in the parameter */
			if (i++ == idx) {
				break;
			}
		} else {		/* if keychain name was given in the parameter */
			cname = xmlGetProp(db_node, BAD_CAST "name");
			if (xmlStrcmp(cname, cname_find) == 0) {
				xmlFree(cname); cname = NULL;
				break;
			}
			xmlFree(cname); cname = NULL;
		}

		db_node = db_node->next;
	}

	return(db_node);
} /* find_keychain() */


xmlNodePtr
find_key(int idx)
{
	xmlNodePtr	db_node = NULL;

	int		i = -1;


	db_node = keychain->children;

	while (db_node  &&  i < idx) {
		if (db_node->type == XML_ELEMENT_NODE)	/* we only care about ELEMENT nodes */
			i++;

		if (i != idx)	/* if we've found it, don't jump to the next sibling */
			db_node = db_node->next;
	}

	return(db_node);
} /* find_key */


char *
parse_newlines(char *line, char dir)		/* dir(direction): "\n" -> '\n' = 0, '\n' -> "\n" = 1 */
{
	char		*ret = NULL;
	int		i = 0, j = 0;
	size_t		nlnum = 0, ret_len = 0;


	if (!line)
		return(strdup(""));


	if (dir) {
		/*
		 * count the number of '\n' characters in the string, and use it later to figure how many bytes
		 * will be the new string, with replaced newline characters.
		 */
		for (i=0; i < (int)strlen(line); i++)
			if (line[i] == '\n')	/* we got a winner... */
				nlnum++;

		ret_len = strlen(line) + nlnum + 1;
	} else {
		/*
		 * count the number of "\n" sequences in the string, and use it later to figure how many bytes
		 * will be the new string, with replaced newline sequences.
		 */
		for (i=0; i < (int)strlen(line); i++)
			if (line[i] == '\\'  &&  line[i+1] == '\\')	/* the "\\n" case. the newline is escaped, so honor it */
				i += 2;					/* skip these. don't count them, because they are not newlines */
			else
			if (line[i] == '\\'  &&  line[i+1] == 'n')	/* we got a winner... */
				nlnum++;

		ret_len = strlen(line) - nlnum + 1;
	}
	ret = malloc(ret_len); malloc_check(ret);


	if (dir) {
		/* replace the real newline characters with newline sequences ("\n"); */
		for (i=0; i < (int)strlen(line); i++) {
			if (line[i] == '\n') {			/* we got a winner... */
				ret[j++] = '\\';		/* replace with NL character */
				ret[j++] = 'n';			/* replace with NL character */
			} else
				ret[j++] = line[i];			/* anything else will just go into the new string */
		}
	} else {
		/* replace the newline sequences with real newline characters ('\n'); */
		for (i=0; i < (int)strlen(line); i++) {
			if (line[i] == '\\'  &&  line[i+1] == '\\') {	/* the "\\n" case. the newline is escaped, so honor it */
				ret[j++] = line[i];			/* copy it as if nothing had happened */
				ret[j++] = line[++i];
			} else
			if (line[i] == '\\'  &&  line[i+1] == 'n' ) {	/* we got a winner... */
				ret[j++] = '\n';			/* replace with NL character */
				i++;					/* skip the 'n' char from "\n" */
			} else
				ret[j++] = line[i];			/* anything else will just go into the new string */
		}
	}

	ret[(long)(ret_len - 1)] = '\0';		/* close that new string safe and secure. */


	return(ret);	/* return the result; we've worked on it hard. */
} /* parse_newlines() */


size_t
digit_length(int idx)
{
	size_t	length = 1;


	while ((idx / 10) != 0) {
		idx /= 10;
		length++;
	}

	return(length);
} /* digit_length() */


#ifdef _READLINE
void
_rl_push_buffer(void)
{
	rl_replace_line((const char *)_rl_helper_var, 0);
	rl_redisplay();
} /* _rl_push_buffer */
#endif


char *
get_random_str(size_t length, char alnum)
{
        int		i = 0;
        int		rnd_file = -1;
 #ifndef _LINUX
        char		*rnd_dev = "/dev/random";
 #else
        char		*rnd_dev = "/dev/urandom";
 #endif
        char		*tmp = NULL;
        ssize_t		ret = -1;
        char		*rnd_str = NULL;
 
 
	rnd_file = open(rnd_dev, O_RDONLY);
	if (rnd_file < 0) {
		printf("Error opening %s!", rnd_dev);
		perror("open()");
		return(NULL);
	}


	rnd_str = malloc((size_t)length + 1); malloc_check(rnd_str);
	tmp = malloc(1); malloc_check(tmp);

	read(rnd_file, tmp, 1);
	for (i=0; i < (int)length; i++) {
		if (alnum)      /* only alphanumeric was requested */
			while ( (*tmp < 65  ||  *tmp > 90)  &&
				(*tmp < 97  ||  *tmp > 122)  &&
				(*tmp < 48  ||  *tmp > 57)) {

				ret = read(rnd_file, tmp, 1);
				if (ret < 0) {
					perror("read(random device)");
					return(NULL);
				}
			}
		else
			/* give anything printable */
			while (*tmp < 33  ||  *tmp > 126) {

				ret = read(rnd_file, tmp, 1);
				if (ret < 0) {
					perror("read(random device)");
					return(NULL);
				}
			}

		rnd_str[i] = *tmp;              /* store the value */
		*tmp = '\0';            /* reset the value */
	}

	free(tmp); tmp = NULL;

	rnd_str[(long)length] = '\0';

	close(rnd_file);


	return(rnd_str);
} /* get_random_str() */
