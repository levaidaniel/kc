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


#ifndef _LINUX
#include <fcntl.h>
#include <readpassphrase.h>
#else
#include <sys/file.h>
#include <bsd/readpassphrase.h>
#endif

#include "common.h"


extern xmlNodePtr	keychain;

extern BIO		*bio_cipher;
extern char		*cipher_mode;

extern unsigned char	salt[17], iv[17], key[128];

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


xmlChar *
parse_randoms(xmlChar *line)
{
	xmlChar		*ret = NULL;
	char		*rand_str = NULL;
	int		i = 0, j = 0;
	size_t		ret_len = 0;


	if (!line)
		return(xmlStrdup(BAD_CAST ""));


	/*
	 * count the number of "\r" and "\R" sequences in the string, and use it later to figure how many bytes
	 * will be the new string, with replaced random sequences.
	 */
	for (i=0; i < (int)xmlStrlen(line); i++) {
		if (line[i] == '\\') {		/* the "\\r" or "\\R" case. the sequence is escaped, so honor it */
			if (line[i+1] == '\\') {		/* the "\\r" or "\\R" case. the sequence is escaped, so honor it */
				i += 2;
				ret_len += 3;
			} else if (line[i+1] == 'r'  ||  line[i+1] == 'a') {		/* we got a winner... "\r" and "\a" is equal to 2 random characters */
				i++;
				ret_len += 2;
			} else if (line[i+1] == 'R'  ||  line[i+1] == 'A') {		/* we got a winner... "\R" and "\A" is equal to 4 random characters */
				i++;
				ret_len += 4;
			} else
				ret_len++;						/* anything else will just go into the new string */
		} else
			ret_len++;						/* anything else will just go into the new string */
	}
	ret_len++;	/* take the closing NUL into account */
	ret = malloc(ret_len); malloc_check(ret);


	/* replace the random sequences with real random characters */
	for (i=0; i < (int)xmlStrlen(line); i++) {
		if (line[i] == '\\') {	/* got an escape character, we better examine it... */
			if (line[i+1] == '\\') {	/* the "\\r" or "\\R" case. the sequence is escaped, so honor it */
				ret[j++] = line[i];	/* copy it as if nothing had happened */
				ret[j++] = line[++i];
			} else if (	(line[i+1] == 'r'  ||  line[i+1] == 'a')  ||
					(line[i+1] == 'R'  ||  line[i+1] == 'A')) {	/* we got a winner... "\r" and "\a" is equal to 2, and "\R" and "\A" is equal to 4 random characters */

				/* replace with random characters */

				if (line[i+1] == 'r'  ||  line[i+1] == 'R')
					rand_str = get_random_str(4, 0);
				else if (line[i+1] == 'a'  ||  line[i+1] == 'A')	/* generate only alphanumeric characters */
					rand_str = get_random_str(4, 1);

				if (rand_str) {
					ret[j++] = rand_str[0];
					ret[j++] = rand_str[1];

					if (line[i+1] == 'R'  ||  line[i+1] == 'A') {
						ret[j++] = rand_str[2];
						ret[j++] = rand_str[3];
					}

					free(rand_str);
				} else
					puts("Random number generation failure!");

				i++;			/* skip the 'r' or 'R' char from "\r" or "\R" */
			} else
				ret[j++] = line[i];	/* anything else will just go into the new string */
		} else
			ret[j++] = line[i];		/* anything else will just go into the new string */
	}

	ret[(long)(ret_len - 1)] = '\0';	/* close that new string safe and secure. */


	return(ret);	/* return the result; we've worked hard on it. */
} /* parse_randoms() */


char
password_read(char **pass1)
{
	/*
	 * returns:
	 * -1 mismatch
	 *  0 cancel
	 *  1 ok
	 */

	char	*pass2 = NULL;


	*pass1 = malloc(PASSWORD_MAXLEN + 1); malloc_check(*pass1);
	readpassphrase("New password (empty to cancel): ", *pass1, PASSWORD_MAXLEN + 1, RPP_ECHO_OFF | RPP_REQUIRE_TTY);
	if (!strlen(*pass1)) {
		free(*pass1); *pass1 = NULL;
		puts("canceled.");
		return(0);
	}

	pass2 = malloc(PASSWORD_MAXLEN + 1); malloc_check(pass2);
	readpassphrase("New password again (empty to cancel): ", pass2, PASSWORD_MAXLEN + 1, RPP_ECHO_OFF | RPP_REQUIRE_TTY);
	if (!strlen(pass2)) {
		free(*pass1); *pass1 = NULL;
		free(pass2); pass2 = NULL;
		puts("canceled.");
		return(0);
	}

	if (strcmp(*pass1, pass2) != 0) {
		free(*pass1); *pass1 = NULL;
		free(pass2); pass2 = NULL;
		puts("Passwords mismatch.");
		return(-1);
	}

	free(pass2); pass2 = NULL;

	return(1);
} /* password_read() */


void
kc_gen_crypt_params(int flags, char *pass) 
{
	char	*rand_str = NULL;


	if ((flags & KC_GENERATE_IV)) {
		rand_str = get_random_str(sizeof(iv) - 1, 0);
		if (!rand_str) {
			puts("IV generation failure!");
			return;
		}
		strlcpy((char *)iv, rand_str, sizeof(iv));
		free(rand_str);

		if (getenv("KC_DEBUG"))
			printf("iv='%s'\n", iv);
	}

	if ((flags & KC_GENERATE_SALT)) {
		rand_str = get_random_str(sizeof(salt) - 1, 0);
		if (!rand_str) {
			puts("Salt generation failure!");
			return;
		}
		strlcpy((char *)salt, rand_str, sizeof(salt));
		free(rand_str);

		if (getenv("KC_DEBUG"))
			printf("salt='%s'\n", salt);
	}

	if ((flags & KC_GENERATE_KEY)) {
		if (getenv("KC_DEBUG"))
			printf("generating new key from pass and salt.\n");

		/* generate a proper key for encoding/decoding BIO */
		PKCS5_PBKDF2_HMAC_SHA1(pass, (int)strlen(pass), salt, sizeof(salt), 5000, 128, key);
	}
} /* generate_iv_salt_key() */


void
kc_set_cipher(int enc)
{
	/* reconfigure encoding with the newly generated key and IV */
	if (strcmp(cipher_mode, "cfb128") == 0) {
		if (getenv("KC_DEBUG"))
			printf("using cipher mode: %s\n", cipher_mode);
		BIO_set_cipher(bio_cipher, EVP_aes_256_cfb128(), key, iv, enc);
	} else if (strcmp(cipher_mode, "ofb") == 0) {
		if (getenv("KC_DEBUG"))
			printf("using cipher mode: %s\n", cipher_mode);
		BIO_set_cipher(bio_cipher, EVP_aes_256_ofb(), key, iv, enc);
	} else {	/* the default is CBC */
		if (getenv("KC_DEBUG"))
			printf("using default cipher mode: %s\n", cipher_mode);
		BIO_set_cipher(bio_cipher, EVP_aes_256_cbc(), key, iv, enc);
	}
} /* kc_set_cipher() */
