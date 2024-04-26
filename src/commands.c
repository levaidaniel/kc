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
#include "ssha.h"
#ifdef _HAVE_YUBIKEY
#include "ykchalresp.h"
#endif

#ifdef BSD
#include <fcntl.h>
#include <readpassphrase.h>
#endif

#ifdef __OpenBSD__
#include <util.h>
#endif

#if defined(__linux__)  ||  defined(__CYGWIN__)
#include <sys/file.h>
#include <bsd/readpassphrase.h>
#include <stdint.h>
#endif

#ifdef _BUNDLED_BCRYPT
#include "bcrypt/bcrypt_pbkdf.h"
#endif

#ifdef _HAVE_LIBSCRYPT
#include <libscrypt.h>
#endif

#ifdef _HAVE_YUBIKEY
#include "ykchalresp.h"
#endif

extern xmlNodePtr	keychain;

extern char		batchmode;

#ifdef _READLINE
xmlChar			*_rl_helper_var = NULL;
#endif


xmlNodePtr
find_keychain(const xmlChar *cname_find, unsigned char name)	/* name: 1 = keychain's name takes priority over its index number */
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*cname = NULL;

	char			*inv = NULL;
	unsigned long int	idx = 0, i = 0;


	if (xmlStrlen(cname_find) == 0)
		return(NULL);

	/* check if we got a number */
	errno = 0;
	idx = strtoul((const char *)cname_find, &inv, 10);

	/* If we didn't get a number.
	 * name == 1 means forced searching for the name of the keychain, instead of the index number.
	 *
	 * Because there is no way to tell if the number was negative with strtoul(),
	 * check for the minus sign. This is sooo great...
	 */
	if (inv[0] != '\0'  ||  errno != 0  ||  cname_find[0] == '-')
		name = 1;


	db_node = keychain->parent->children;

	while (db_node  &&  i < ITEMS_MAX) {
		if (db_node->type != XML_ELEMENT_NODE) {	/* we only care about ELEMENT nodes */
			db_node = db_node->next;
			continue;
		}

		if (name) {		/* if we are searching for the keychain's name */
			cname = xmlGetProp(db_node, BAD_CAST "name");
			if (xmlStrcmp(cname, cname_find) == 0) {
				xmlFree(cname); cname = NULL;
				break;
			}
			xmlFree(cname); cname = NULL;
		} else {		/* if we are searching for the nth index */
			if (i++ == idx)
				break;
		}

		db_node = db_node->next;
	}

	return(db_node);
} /* find_keychain() */


xmlNodePtr
find_key(const unsigned long int idx)
{
	xmlNodePtr	db_node = NULL;

	unsigned long int	i = 0;


	db_node = keychain->children;
	while (db_node  &&  i < ITEMS_MAX) {
		if (db_node->type == XML_ELEMENT_NODE)	/* we only care about ELEMENT nodes */
			if (i++ == idx)
				break;

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
get_random_str(const unsigned int length, const unsigned char mode)
{
	unsigned int	i = 0;
	int		rnd_file = -1;
#if defined(__linux__)  ||  defined(__CYGWIN__)
	char		*rnd_dev = "/dev/urandom";
#else
	char		*rnd_dev = "/dev/random";
#endif
	char		*tmp = NULL;
	ssize_t		ret = -1;
	char		*rnd_str = NULL;


	rnd_file = open(rnd_dev, O_RDONLY);
	if (rnd_file < 0) {
		dprintf(STDERR_FILENO, "ERROR: Error opening %s!\n", rnd_dev);
		perror("ERROR: open()");
		return(NULL);
	}


	rnd_str = malloc(length + 1); malloc_check(rnd_str);
	tmp = malloc(1); malloc_check(tmp);

	read(rnd_file, tmp, 1);
	for (i=0; i < length; i++) {
		switch (mode) {
			case 0:
			/* only alphanumeric was requested */
				while ( (*tmp < 65  ||  *tmp > 90)  &&
					(*tmp < 97  ||  *tmp > 122)  &&
					(*tmp < 48  ||  *tmp > 57)) {

					ret = read(rnd_file, tmp, 1);
					if (ret <= 0) {
						perror("ERROR: read(random device)");

						free(rnd_str); rnd_str = NULL;
						free(tmp); tmp = NULL;
						return(NULL);
					}
				}
			break;
			case 1:
			/* give anything printable */
				while (*tmp < 33  ||  *tmp > 126) {

					ret = read(rnd_file, tmp, 1);
					if (ret <= 0) {
						perror("ERROR: read(random device)");

						free(rnd_str); rnd_str = NULL;
						free(tmp); tmp = NULL;
						return(NULL);
					}
				}
			break;
			case 2:
			/* give anything */
				ret = read(rnd_file, tmp, 1);
				if (ret <= 0) {
					perror("ERROR: read(random device)");

					free(rnd_str); rnd_str = NULL;
					free(tmp); tmp = NULL;
					return(NULL);
				}
			break;
			default:
			/* invalid calling of the function */
				free(rnd_str); rnd_str = NULL;
				free(tmp); tmp = NULL;
				return(NULL);
			break;
		}

		rnd_str[i] = *tmp;	/* store the value */
		*tmp = '\0';		/* reset the value */
	}

	free(tmp); tmp = NULL;

	rnd_str[length] = '\0';

	close(rnd_file);


	return(rnd_str);
} /* get_random_str() */


xmlChar *
parse_randoms(const xmlChar *line)
{
	xmlChar	*ret = NULL;
	char	*rand_str = NULL;
	int	i = 0, j = 0, ret_len = 0, line_len = 0;


	if (!line) {
		ret = xmlStrdup(BAD_CAST ""); malloc_check(ret);
		return(ret);
	}


	line_len = xmlStrlen(line);
	/*
	 * count the number of "\r" and "\R" sequences in the string, and use it later to figure how many bytes
	 * will be the new string, with replaced random sequences.
	 */
	for (i=0; i < line_len; i++) {
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
	for (i=0; i < line_len; i++) {
		if (line[i] == '\\') {	/* got an escape character, we better examine it... */
			if (line[i+1] == '\\') {	/* the "\\r" or "\\R" case. the sequence is escaped, so honor it */
				ret[j++] = line[i];	/* copy it as if nothing had happened */
				ret[j++] = line[++i];
			} else if (	(line[i+1] == 'r'  ||  line[i+1] == 'a')  ||
					(line[i+1] == 'R'  ||  line[i+1] == 'A')) {	/* we got a winner... "\r" and "\a" is equal to 2, and "\R" and "\A" is equal to 4 random characters */

				/* replace with random characters */

				if (line[i+1] == 'r'  ||  line[i+1] == 'R')
					rand_str = get_random_str(4, 1);
				else if (line[i+1] == 'a'  ||  line[i+1] == 'A')	/* generate only alphanumeric characters */
					rand_str = get_random_str(4, 0);

				if (rand_str) {
					ret[j++] = rand_str[0];
					ret[j++] = rand_str[1];

					if (line[i+1] == 'R'  ||  line[i+1] == 'A') {
						ret[j++] = rand_str[2];
						ret[j++] = rand_str[3];
					}

					free(rand_str); rand_str = NULL;
				} else
					dprintf(STDERR_FILENO, "ERROR: Random number generation failure!\n");

				i++;			/* skip the 'r' or 'R' char from "\r" or "\R" */
			} else
				ret[j++] = line[i];	/* anything else will just go into the new string */
		} else
			ret[j++] = line[i];		/* anything else will just go into the new string */
	}

	ret[(ret_len - 1)] = '\0';	/* close that new string safe and secure. */


	return(ret);	/* return the result; we've worked hard on it. */
} /* parse_randoms() */


/*
 * Get the idx'th line from the value.
 */
xmlChar *
get_line(const xmlChar *value_nl, const unsigned long int idx)
{
	xmlChar	*line = NULL;

	unsigned long int	nl = 1, pos = 0, tmp = 0, length = 0;


	length = xmlStrlen(value_nl);

	/* find out the start position (pos) of the requested line number (idx) */
	for (pos = 0;(pos < length)  &&  (idx > nl); pos++) {
		if (value_nl[pos] == '\n') {	/* we've found a newline */
			nl++;
		}
	}

	tmp = pos;
	length = 0;
	/* count the requested line length */
	while (value_nl[pos] != '\n'  &&  value_nl[pos] != '\0') {
		length++;
		pos++;
	}

	pos = tmp;
	tmp = 0;
	line = malloc(length + 1); malloc_check(line);
	/* copy out the requested line */
	while (value_nl[pos] != '\n'  &&  value_nl[pos] != '\0'  &&  tmp < length) {
		line[tmp++] = value_nl[pos++];
	}
	line[length] = '\0';

	return(line);
} /* get_line() */


xmlChar *
parse_newlines(const xmlChar *line, const unsigned char dir)		/* dir(direction): "\n" -> '\n' = 0, '\n' -> "\n" = 1 */
{
	xmlChar		*ret = NULL;
	int		i = 0, j = 0, nlnum = 0, ret_len = 0, line_len = 0;


	if (!line) {
		ret = xmlStrdup(BAD_CAST ""); malloc_check(ret);
		return(ret);
	}

	line_len = xmlStrlen(line);

	if (dir) {
		/*
		 * count the number of '\n' characters in the string, and use it later to figure how many bytes
		 * will be the new string, with replaced newline characters.
		 */
		for (i=0; i < line_len; i++)
			if (line[i] == '\n')	/* we got a winner... */
				nlnum++;

		ret_len = line_len + nlnum + 1;
	} else {
		/*
		 * count the number of "\n" sequences in the string, and use it later to figure how many bytes
		 * will be the new string, with replaced newline sequences.
		 */
		for (i=0; i < line_len; i++) {
			if (line[i] == '\\') {	/* got an escape character, we better examine it... */
				if (line[i+1] == '\\')	/* the "\\n" case. the newline is escaped, so honor it */
					i += 2;		/* skip these. don't count them, because they are not newlines */
				else if (line[i+1] == 'n')	/* we got a winner... */
					nlnum++;
			}
		}

		ret_len = line_len - nlnum + 1;
	}
	ret = malloc(ret_len); malloc_check(ret);


	if (dir) {
		/* replace the real newline characters with newline sequences ("\n"); */
		for (i=0; i < line_len; i++) {
			if (line[i] == '\n') {			/* we got a winner... */
				ret[j++] = '\\';		/* replace with NL character */
				ret[j++] = 'n';			/* replace with NL character */
			} else
				ret[j++] = line[i];			/* anything else will just go into the new string */
		}
	} else {
		/* replace the newline sequences with real newline characters ('\n'); */
		for (i=0; i < line_len; i++) {
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

	ret[ret_len - 1] = '\0';		/* close that new string safe and secure. */


	return(ret);	/* return the result; we've worked hard on it. */
} /* parse_newlines() */


unsigned long int
count_elements(xmlNodePtr db_node)
{
	unsigned long int	count = 0;


	while (db_node  &&  count < ITEMS_MAX) {
		if (db_node->type == XML_ELEMENT_NODE)	/* we only care about ELEMENT nodes */
			count++;

		db_node = db_node->next;
	}


	return(count);
} /* count_items() */


/* Create a local argv/argc pair that contains the given
 * options (to be later used with getopt(3)).
 */
void
larg(char *line, char ***largv, int *largc)
{
	char	*arg = NULL;


	*largv = malloc(sizeof(char *)); malloc_check(*largv);
	(*largv)[*largc] = strdup(strtok(line, " ")); malloc_check((*largv)[*largc]);
	while ((arg = strtok(NULL, " "))) {
		*largv = realloc(*largv, (++(*largc) + 1) * sizeof(char *)); malloc_check(*largv);
		(*largv)[*largc] = strdup(arg); malloc_check((*largv)[*largc]);
	}
	*largv = realloc(*largv, (++(*largc) + 1) * sizeof(char *)); malloc_check(*largv);
	(*largv)[*largc] = NULL;
} /* larg() */


char
kc_password_read(struct db_parameters *db_params, const unsigned char new)
{
	/*
	 * returns:
	 * -1 mismatch
	 *  0 cancel
	 *  1 ok
	 */

	char	*pass1 = NULL, *pass2 = NULL;
	int	rpp_flags = 0;


	rpp_flags = RPP_ECHO_OFF;
	if (batchmode == 1)
		rpp_flags |= RPP_STDIN;
	else
		rpp_flags |= RPP_REQUIRE_TTY;


	pass1 = malloc(PASSWORD_MAXLEN + 1); malloc_check(pass1);

	if (new)
		readpassphrase("New password (empty to cancel): ", pass1, PASSWORD_MAXLEN + 1, rpp_flags);
	else
		readpassphrase("Password: ", pass1, PASSWORD_MAXLEN + 1, rpp_flags);

	if (new) {
		if (!strlen(pass1)) {
			if (pass1)
				memset(pass1, '\0', PASSWORD_MAXLEN + 1);
			free(pass1); pass1 = NULL;

			puts("Canceled.");
			return(0);
		}

		pass2 = malloc(PASSWORD_MAXLEN + 1); malloc_check(pass2);
		readpassphrase("New password again (empty to cancel): ", pass2, PASSWORD_MAXLEN + 1, rpp_flags);
		if (!strlen(pass2)) {
			if (pass1)
				memset(pass1, '\0', PASSWORD_MAXLEN + 1);
			free(pass1); pass1 = NULL;

			if (pass2)
				memset(pass2, '\0', PASSWORD_MAXLEN + 1);
			free(pass2); pass2 = NULL;

			puts("Canceled.");
			return(0);
		}

		if (strcmp(pass1, pass2) != 0) {
			if (pass1)
				memset(pass1, '\0', PASSWORD_MAXLEN + 1);
			free(pass1); pass1 = NULL;

			if (pass2)
				memset(pass2, '\0', PASSWORD_MAXLEN + 1);
			free(pass2); pass2 = NULL;

			puts("Passwords mismatch.");
			return(-1);
		}

		if (pass2)
			memset(pass2, '\0', PASSWORD_MAXLEN + 1);
		free(pass2); pass2 = NULL;
	}

	if (pass1 == NULL  ||  !strlen(pass1)) {
		puts("Password is of zero length!");
		return(-1);
	}

	db_params->pass_len = strlen(pass1);
	db_params->pass = malloc(db_params->pass_len); malloc_check(db_params->pass);
	memcpy(db_params->pass, pass1, db_params->pass_len);

	if (pass1)
		memset(pass1, '\0', PASSWORD_MAXLEN + 1);
	free(pass1); pass1 = NULL;


	return(1);
} /* kc_password_read() */


char
kc_crypt_iv_salt(struct db_parameters *db_params)
{
	char	*iv_tmp = NULL, *salt_tmp = NULL;
	unsigned int	i = 0;

	EVP_MD_CTX	*mdctx = NULL;
	unsigned char	digested[EVP_MAX_MD_SIZE];
	unsigned int	digested_len = 0;
	char		hex_tmp[3];


	iv_tmp = get_random_str(IV_LEN, 2);
	if (!iv_tmp) {
		dprintf(STDERR_FILENO, "ERROR: IV generation failure!\n");
		return(0);
	}

	/* Setup the digest context */
	mdctx = EVP_MD_CTX_new();
	if (!mdctx) {
		dprintf(STDERR_FILENO, "ERROR: Could not create digest context for IV!\n");

		free(iv_tmp); iv_tmp = NULL;
		return(0);
	}
	EVP_DigestInit_ex(mdctx, EVP_sha512(), NULL);
	EVP_DigestUpdate(mdctx, iv_tmp, IV_LEN);
	EVP_DigestFinal_ex(mdctx, digested, &digested_len);
	EVP_MD_CTX_free(mdctx);
	free(iv_tmp); iv_tmp = NULL;

	/* Print the binary digest in hex as characters (numbers, effectively)
	 * into the ..->iv variable.
	 */
	for (i = 0; i < digested_len; i++) {
		snprintf(hex_tmp, 3, "%02x", digested[i]);

		if (!i) {
			if (strlcpy(	(char *)db_params->iv, hex_tmp,
					IV_DIGEST_LEN + 1)
					>= IV_DIGEST_LEN + 1)
				return(0);
		} else {
			if (strlcat(	(char *)db_params->iv, hex_tmp,
					IV_DIGEST_LEN + 1)
					>= IV_DIGEST_LEN + 1)
				return(0);
		}
	}

	if (getenv("KC_DEBUG"))
		printf("%s(): iv='%s'\n", __func__, db_params->iv);


	salt_tmp = get_random_str(SALT_LEN, 2);
	if (!salt_tmp) {
		dprintf(STDERR_FILENO, "ERROR: Salt generation failure!\n");
		return(0);
	}

	/* Setup the digest context */
	mdctx = EVP_MD_CTX_new();
	if (!mdctx) {
		dprintf(STDERR_FILENO, "ERROR: Could not create digest context for salt!\n");

		free(salt_tmp); salt_tmp = NULL;
		return(0);
	}
	EVP_DigestInit_ex(mdctx, EVP_sha512(), NULL);
	EVP_DigestUpdate(mdctx, salt_tmp, SALT_LEN);
	EVP_DigestFinal_ex(mdctx, digested, &digested_len);
	EVP_MD_CTX_free(mdctx);
	free(salt_tmp); salt_tmp = NULL;

	/* Print the binary digest in hex as characters (numbers, effectively)
	 * into the ..->salt variable.
	 */
	for (i = 0; i < digested_len; i++) {
		snprintf(hex_tmp, 3, "%02x", digested[i]);

		if (!i) {
			if (strlcpy(	(char *)db_params->salt, hex_tmp,
					SALT_DIGEST_LEN + 1)
					>= SALT_DIGEST_LEN + 1)
				return(0);
		} else {
			if (strlcat(	(char *)db_params->salt, hex_tmp,
					SALT_DIGEST_LEN + 1)
					>= SALT_DIGEST_LEN + 1)
				return(0);
		}
	}

	if (getenv("KC_DEBUG"))
		printf("%s(): salt='%s'\n", __func__, db_params->salt);


	return(1);
} /* kc_crypt_iv_salt() */


char
kc_crypt_pass(struct db_parameters *db_params, const unsigned char newdb)
{
	int		pass_file = -1, pos = 0;
	ssize_t		ret = -1;

	struct stat	st;


	/* Get the password one way or another */
	if (db_params->pass_filename) {	/* we were given a password file name */
		if (getenv("KC_DEBUG"))
			printf("%s(): opening password file\n", __func__);

		if (stat(db_params->pass_filename, &st) == 0) {
			if (!S_ISLNK(st.st_mode)  &&  !S_ISREG(st.st_mode)) {
				dprintf(STDERR_FILENO, "ERROR: '%s' is not a regular file or a link!\n", db_params->pass_filename);
				return(-1);
			}
		} else {
			perror("ERROR: stat(password file)");
			return(-1);
		}

		/* read in the password from the specified file */
		pass_file = open(db_params->pass_filename, O_RDONLY);
		if (pass_file < 0) {
			perror("ERROR: open(password file)");
			return(-1);
		}

		db_params->pass = malloc(PASSWORD_MAXLEN + 1); malloc_check(db_params->pass);
		pos = 0;
		/* We read PASSWORD_MAXLEN plus one byte, to see if the password in the
		 * password file is longer than PASSWORD_MAXLEN.
		 */
		do {
			ret = read(pass_file, db_params->pass + pos, PASSWORD_MAXLEN + 1 - pos);
			pos += ret;
		} while (ret > 0  &&  pos < PASSWORD_MAXLEN + 1);

		if (ret < 0) {
			perror("ERROR: read(password file)");
			return(-1);
		}
		if (pos == 0) {
			dprintf(STDERR_FILENO, "ERROR: Password file must not be empty!\n");
			return(-1);
		}

		if (pos > PASSWORD_MAXLEN) {
			printf("WARNING: the password in '%s' is longer than the maximum allowed length (%d bytes) of a password, and it was truncated to %d bytes!\n\n", db_params->pass_filename, PASSWORD_MAXLEN, PASSWORD_MAXLEN);
			pos = PASSWORD_MAXLEN;
		}

		db_params->pass_len = pos;

#ifdef _HAVE_YUBIKEY
		if (db_params->yk) {
			if (!db_params->yk_password) {
				dprintf(STDERR_FILENO, "ERROR: 'password' option is not specified for YubiKey parameter while trying to use a password file!\n");
				return(-1);
			}

			if (!kc_ykchalresp(db_params)) {
				dprintf(STDERR_FILENO, "ERROR: Error while doing YubiKey challenge-response!\n");
				return(-1);
			}
		}
#endif
		if (strlen(db_params->ssha_type)) {
			if (strlen(db_params->ssha_type)  &&  !db_params->ssha_password) {
				dprintf(STDERR_FILENO, "ERROR: 'password' option is not specified for SSH agent parameter while trying to use a password file!\n");
				return(-1);
			}

			if (!kc_ssha_get_password(db_params))
				return(-1);
		}

		if (close(pass_file) < 0)
			perror("ERROR: close(password file)");
	} else {
		if (	db_params->ssha_password  ||
			db_params->yk_password  ||
			(!db_params->yk  &&  !strlen(db_params->ssha_type))
		) {
			if (getenv("KC_DEBUG"))
				printf("%s(): getting a password for the database\n", __func__);

			/* ask for the new password */
			if (kc_password_read(db_params, newdb) != 1)
				return(-1);
		}

#ifdef _HAVE_YUBIKEY
		if (db_params->yk) {
			/* use a YubiKey to generate the password */
			if (!kc_ykchalresp(db_params)) {
				dprintf(STDERR_FILENO, "ERROR: Error while doing YubiKey challenge-response!\n");
				return(-1);
			}
		}
#endif
		if (strlen(db_params->ssha_type)) {
			/* use SSH agent to generate the password */
			if (!kc_ssha_get_password(db_params))
				return(-1);
		}
	}

	return(1);
} /* kc_crypt_pass() */


char
kc_crypt_key(struct db_parameters *db_params)
{
	char		*inv = NULL;

	/* only for Argon2id */
	EVP_KDF		*kdf_impl = NULL;
	EVP_KDF_CTX	*kdf_ctx = NULL;
	OSSL_PARAM	kdf_opts[6];
	uint32_t	argon2id_lanes = KC_ARGON2ID_LANES;
	uint32_t	argon2id_memcost = KC_ARGON2ID_MEMCOST;


	if (getenv("KC_DEBUG")) {
		printf("%s(): generating new key from pass (len:%zd) and salt.\n", __func__, db_params->pass_len);
		printf("%s(): using %s based KDF (%lu iterations)\n", __func__, db_params->kdf, db_params->kdf_reps);
		printf("%s(): encryption key length is %lu bytes\n", __func__, db_params->key_len);
	}

	if (!(db_params->key)) {
		db_params->key = malloc(db_params->key_len); malloc_check(db_params->key);
	} else {
		dprintf(STDERR_FILENO, "ERROR: Failed to allocate space for new key; already occupied!\n");
		return(0);
	}

	/* Generate a proper key from the user's password */
	if (strcmp(db_params->kdf, "sha512") == 0) {
		if (!PKCS5_PBKDF2_HMAC(db_params->pass, db_params->pass_len,
			db_params->salt, SALT_DIGEST_LEN + 1,
			db_params->kdf_reps, EVP_sha512(),
			db_params->key_len, db_params->key))
		{
			dprintf(STDERR_FILENO, "ERROR: Failed to generate a key from the password!\n");
			if (getenv("KC_DEBUG"))
				printf("%s(): PKCS5_PBKDF2_HMAC() error\n", __func__);

			return(0);
		}
	} else if (strcmp(db_params->kdf, "sha3") == 0) {
		if (!PKCS5_PBKDF2_HMAC(db_params->pass, db_params->pass_len,
			db_params->salt, SALT_DIGEST_LEN + 1,
			db_params->kdf_reps, EVP_sha3_512(),
			db_params->key_len, db_params->key))
		{
			dprintf(STDERR_FILENO, "ERROR: Failed to generate a key from the password!\n");
			if (getenv("KC_DEBUG"))
				printf("%s(): PKCS5_PBKDF2_HMAC() error\n", __func__);

			return(0);
		}
	} else if (strcmp(db_params->kdf, "bcrypt") == 0) {
		if (bcrypt_pbkdf(db_params->pass, db_params->pass_len,
			db_params->salt, SALT_DIGEST_LEN + 1,
			db_params->key, db_params->key_len, db_params->kdf_reps) != 0)
		{
			dprintf(STDERR_FILENO, "ERROR: Failed to generate a key from the password!\n");
			if (getenv("KC_DEBUG"))
				printf("%s(): bcrypt_pbkdf() error\n", __func__);

			return(0);
		}
#ifdef _HAVE_ARGON2
	} else if (strcmp(db_params->kdf, "argon2id") == 0) {
		kdf_opts[0] = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_SALT, db_params->salt, SALT_DIGEST_LEN + 1);
		kdf_opts[1] = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_PASSWORD, db_params->pass, db_params->pass_len);
		/* we truncate 'kdf_reps' here from /long/ to /int/ */
		kdf_opts[2] = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ITER, (uint32_t*)&(db_params->kdf_reps));

		/* Option '-1' is memory lanes here */
		if (db_params->first) {
			argon2id_lanes = strtoul(db_params->first, &inv, 10);
			if (inv[0] != '\0') {
				dprintf(STDERR_FILENO, "ERROR: Unable to convert the Argon2 memory lanes parameter.\n");
				return(-1);
			}
		}
		if (getenv("KC_DEBUG"))
			printf("%s(): Argon2id memory lanes: %d\n", __func__, argon2id_lanes);
		kdf_opts[3] = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_LANES, &argon2id_lanes);

		/* Option '-2' is memcost here */
		if (db_params->second) {
			argon2id_memcost = strtoul(db_params->second, &inv, 10);
			if (inv[0] != '\0') {
				dprintf(STDERR_FILENO, "ERROR: Unable to convert the Argon2 memcost parameter.\n");
				return(-1);
			}
		}
		if (getenv("KC_DEBUG"))
			printf("%s(): Argon2id memcost: %d\n", __func__, argon2id_memcost);
		kdf_opts[4] = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_MEMCOST, &argon2id_memcost);

		kdf_opts[5] = OSSL_PARAM_construct_end();


		if ((kdf_impl = EVP_KDF_fetch(NULL, "ARGON2ID", NULL)) == NULL) {
			dprintf(STDERR_FILENO, "ERROR: Unable to fetch Argon2 KDF.\n");
			return(0);
		}
		if ((kdf_ctx = EVP_KDF_CTX_new(kdf_impl)) == NULL) {
			dprintf(STDERR_FILENO, "ERROR: Unable to create Argon2 KDF context.\n");
			return(0);
		}

		if (EVP_KDF_CTX_set_params(kdf_ctx, kdf_opts) != 1) {
			dprintf(STDERR_FILENO, "ERROR: Unable to set parameters for Argon2 KDF.\n");
			return(0);
		}
		if (EVP_KDF_derive(kdf_ctx, db_params->key, db_params->key_len, NULL) != 1) {
			dprintf(STDERR_FILENO, "ERROR: Unable to derive key with Argon2 KDF.\n");
			return(0);
		}
#endif
#ifdef _HAVE_LIBSCRYPT
	} else if (strcmp(db_params->kdf, "scrypt") == 0) {
		if (libscrypt_scrypt((const unsigned char *)db_params->pass, db_params->pass_len,
			db_params->salt, SALT_DIGEST_LEN + 1,
			SCRYPT_N, SCRYPT_r, SCRYPT_p,
			db_params->key, db_params->key_len) != 0)
		{
			dprintf(STDERR_FILENO, "ERROR: Failed to generate a key from the password!\n");
			if (getenv("KC_DEBUG"))
				printf("%s(): libscrypt_scrypt() error\n", __func__);

			return(0);
		}
#endif
	} else {
		printf("Unknown kdf: %s!\n", db_params->kdf);
		return(0);
	}


	return(1);
} /* kc_crypt_key() */


char
kc_crypt_setup(BIO *bio_chain, const unsigned int enc, struct db_parameters *db_params)
{
	const EVP_CIPHER	*(*kc_cipher)(void) = NULL;


	/* extract bio_cipher from bio_chain */
	bio_chain = BIO_find_type(bio_chain, BIO_TYPE_CIPHER);
	if (!bio_chain) {
		dprintf(STDERR_FILENO, "ERROR: Could not find a usable cipher method!\n");

		return(0);
	}


	/* reconfigure {en,de}cryption with the key and IV */
	if (strcmp(db_params->cipher, "aes256") == 0) {
		if (strcmp(db_params->cipher_mode, "cfb") == 0) {
			kc_cipher = &EVP_aes_256_cfb;
		} else if (strcmp(db_params->cipher_mode, "ofb") == 0) {
			kc_cipher = &EVP_aes_256_ofb;
		} else if (strcmp(db_params->cipher_mode, "cbc") == 0) {
			kc_cipher = &EVP_aes_256_cbc;
		} else if (strcmp(db_params->cipher_mode, "ctr") == 0) {
			kc_cipher = &EVP_aes_256_ctr;
		} else {
			dprintf(STDERR_FILENO, "ERROR: Unknown cipher mode: %s!\n", db_params->cipher_mode);
			return(0);
		}
	} else if (strcmp(db_params->cipher, "blowfish") == 0) {
		if (strcmp(db_params->cipher_mode, "cfb") == 0) {
			kc_cipher = &EVP_bf_cfb;
		} else if (strcmp(db_params->cipher_mode, "ofb") == 0) {
			kc_cipher = &EVP_bf_ofb;
		} else if (strcmp(db_params->cipher_mode, "cbc") == 0) {
			kc_cipher = &EVP_bf_cbc;
		} else {
			dprintf(STDERR_FILENO, "ERROR: Unknown cipher mode: %s!\n", db_params->cipher_mode);
			return(0);
		}
	} else if (strcmp(db_params->cipher, "chacha20-poly1305") == 0) {
		kc_cipher = &EVP_chacha20_poly1305;

		/* chacha20-poly1305 doesn't use cipher_mode */
		db_params->cipher_mode = realloc(db_params->cipher_mode, 4); malloc_check(db_params->cipher_mode);
		if (strlcpy(db_params->cipher_mode, "n/a", 4) >= 4) {
			dprintf(STDERR_FILENO, "ERROR: Error while setting up database parameter (cipher mode).\n");
			return(0);
		}
	} else {
		printf("Unknown encryption cipher: %s!\n", db_params->cipher);
		return(0);
	}

	if (BIO_set_cipher(bio_chain, (*kc_cipher)(), db_params->key, db_params->iv, enc) != 1) {
		dprintf(STDERR_FILENO, "ERROR: Could not set cipher to '%s' with mode '%s'!\n", db_params->cipher, db_params->cipher_mode);
		return(0);
	}

	if (getenv("KC_DEBUG"))
		printf("%s(): crypt setup: using cipher %s in %s mode\n", __func__, db_params->cipher, db_params->cipher_mode);


	return(1);
} /* kc_crypt_setup() */


BIO *
kc_setup_bio_chain(const char *db_filename, const unsigned char write)
{
	BIO	*bio_file = NULL;
	BIO	*bio_b64 = NULL;
	BIO	*bio_cipher = NULL;
	BIO	*bio_chain = NULL;


	if (write)
		bio_file = BIO_new_file(db_filename, "w+");
	else
		bio_file = BIO_new_file(db_filename, "r");
	if (!bio_file) {
		perror("ERROR: BIO_new_file()");
		return(NULL);
	}
	BIO_set_close(bio_file, BIO_CLOSE);	/* On ignoring the return value: BIO_set_close() always returns 1, according to the openssl documentation. */
	bio_chain = BIO_push(bio_file, bio_chain);

	bio_b64 = BIO_new(BIO_f_base64());
	if (!bio_b64) {
		perror("ERROR: BIO_new(f_base64)");
		return(NULL);
	}
	bio_chain = BIO_push(bio_b64, bio_chain);

	bio_cipher = BIO_new(BIO_f_cipher());
	if (!bio_cipher) {
		perror("ERROR: BIO_new(f_cipher)");
		return(NULL);
	}
	bio_chain = BIO_push(bio_cipher, bio_chain);

	return(bio_chain);
} /* kc_setup_bio_chain() */


char
kc_db_writer(xmlDocPtr db, BIO *bio_chain, struct db_parameters *db_params)
{
	xmlSaveCtxtPtr	xml_save = NULL;
	xmlBufferPtr	xml_buf = NULL;

	int		ret = 0, remaining = 0;


	xml_buf = xmlBufferCreate();
	xml_save = xmlSaveToBuffer(xml_buf, "UTF-8", XML_SAVE_FORMAT);

	if (xml_save) {
		xmlSaveDoc(xml_save, db);
		xmlSaveFlush(xml_save);
		if (getenv("KC_DEBUG"))
			printf("%s(): xml_buf content:\n'%s'(%d)\n", __func__, xmlBufferContent(xml_buf), (int)xmlBufferLength(xml_buf));
		xmlSaveClose(xml_save);


		/* write the IV */
		if (getenv("KC_DEBUG"))
			printf("%s(): writing IV\n", __func__);

		remaining = IV_DIGEST_LEN;
		while (remaining > 0) {
			ret = write(db_params->db_file, db_params->iv, remaining);

			if (ret < 0) {
				perror("ERROR: writing IV to database file");
				return(0);
			}

			remaining -= ret;

			if (getenv("KC_DEBUG")) {
				printf("%s(): wrote: %d\n", __func__, ret);
				printf("%s(): remaining: %d\n", __func__, remaining);
			}
		}
		/* Write a newline at the end */
		ret = write(db_params->db_file, "\n", 1);
		if (ret != 1) {
			perror("ERROR: writing IV to database file");
			return(0);
		}


		/* write the salt */
		if (getenv("KC_DEBUG"))
			printf("%s(): writing salt\n", __func__);

		remaining = SALT_DIGEST_LEN;
		while (remaining > 0) {
			ret = write(db_params->db_file, db_params->salt, remaining);

			if (ret < 0) {
				perror("ERROR: writing salt to database file");
				return(0);
			}

			remaining -= ret;

			if (getenv("KC_DEBUG")) {
				printf("%s(): wrote: %d\n", __func__, ret);
				printf("%s(): remaining: %d\n", __func__, remaining);
			}
		}
		/* Write a newline at the end */
		ret = write(db_params->db_file, "\n", 1);
		if (ret != 1) {
			perror("ERROR: writing salt to database file");
			return(0);
		}


		/* we must reset the cipher BIO to work after subsequent calls to kc_db_writer() */
		if (BIO_reset(bio_chain) != 0) {
			if (getenv("KC_DEBUG"))
				printf("%s(): BIO_reset() error\n", __func__);

			return(0);
		}

		/* seek after the IV and salt */
		if (BIO_seek(bio_chain, IV_DIGEST_LEN + SALT_DIGEST_LEN + 2) != 0) {
			if (getenv("KC_DEBUG"))
				printf("%s(): BIO_seek() error\n", __func__);

			return(0);
		}

		/* Write the database */
		if (getenv("KC_DEBUG"))
			printf("%s(): writing database\n", __func__);

		remaining = xmlBufferLength(xml_buf);
		while (remaining > 0) {
			ret = BIO_write(bio_chain, xmlBufferContent(xml_buf), remaining);

			if (ret <= 0) {
				if (BIO_should_retry(bio_chain)) {
					if (getenv("KC_DEBUG"))
						printf("%s(): write delay\n", __func__);

					sleep(1);
					continue;
				} else {
					if (getenv("KC_DEBUG"))
						printf("%s(): BIO_write() error (don't retry)\n", __func__);

					break;
				}
			}

			remaining -= ret;

			if (getenv("KC_DEBUG")) {
				printf("%s(): wrote: %d\n", __func__, ret);
				printf("%s(): remaining: %d\n", __func__, remaining);
			}
		}


		/* Flush the written stuff */
		do {
			if (BIO_flush(bio_chain) == 1) {
				if (getenv("KC_DEBUG"))
					printf("%s(): flushed bio_chain\n", __func__);
			} else {
				if (BIO_should_retry(bio_chain)) {
					if (getenv("KC_DEBUG"))
						printf("%s(): flush delay\n", __func__);

					sleep(1);
					continue;
				} else {
					if (getenv("KC_DEBUG"))
						printf("%s(): BIO_should_retry() is false\n", __func__);

					break;
				}
			}
		} while (BIO_wpending(bio_chain) > 0);

		if (getenv("KC_DEBUG"))
			printf("%s(): db_file size -> %d\n", __func__, BIO_tell(bio_chain));

		xmlBufferFree(xml_buf);

		return(1);
	} else {
		if (getenv("KC_DEBUG"))
			printf("%s(): xmlSaveToBuffer() error\n", __func__);

		return(0);
	}
} /* kc_db_writer() */


char
kc_validate_xml(xmlDocPtr db, char legacy)
{
	xmlParserInputBufferPtr	buf = NULL;
	xmlDtdPtr		dtd = NULL;
	xmlValidCtxtPtr		valid_ctx = NULL;


	if (legacy)
		buf = xmlParserInputBufferCreateMem(KC_DTD_LEGACY, sizeof(KC_DTD_LEGACY), XML_CHAR_ENCODING_NONE);
	else
		buf = xmlParserInputBufferCreateMem(KC_DTD, sizeof(KC_DTD), XML_CHAR_ENCODING_NONE);
	if (!buf) {
		if (getenv("KC_DEBUG"))
			xmlGenericError(xmlGenericErrorContext, "ERROR: Could not allocate buffer for DTD.\n");

		return(0);
	}

	/* XXX xmlIOParseDTD() is supposed to free the ParserInputBuffer context.
	 * https://mail.gnome.org/archives/xml/2001-July/msg00035.html
	 */
	dtd = xmlIOParseDTD(NULL, buf, XML_CHAR_ENCODING_NONE);
	if (!dtd) {
		if (getenv("KC_DEBUG"))
			xmlGenericError(xmlGenericErrorContext, "ERROR: Could not parse kc DTD.\n");

		xmlFreeParserInputBuffer(buf);
		return(0);
	}

	valid_ctx = xmlNewValidCtxt();
	if (!valid_ctx ) {
		if (getenv("KC_DEBUG"))
			xmlGenericError(xmlGenericErrorContext, "ERROR: Could not allocate a new validation context.\n");

		xmlFreeDtd(dtd);
		return(0);
	}


	if (!xmlValidateDtd(valid_ctx, db, dtd)) {
		if (getenv("KC_DEBUG"))
			xmlGenericError(xmlGenericErrorContext, "ERROR: Validation failed against kc DTD.\n");

		xmlFreeValidCtxt(valid_ctx);
		xmlFreeDtd(dtd);
		return(0);
	}


	xmlFreeValidCtxt(valid_ctx);
	xmlFreeDtd(dtd);


	return(1);
} /* kc_validate_xml() */


int
kc_db_reader(char **buf, BIO *bio_chain)
{
	int		pos = 0, buf_size = 0, read_size = 4096;
	ssize_t		ret = -1;


	/* Seek after the IV and salt */
	if (getenv("KC_DEBUG"))
		printf("%s(): skipping over IV and salt while reading db\n", __func__);

	BIO_seek(bio_chain, IV_DIGEST_LEN + SALT_DIGEST_LEN + 2);

	if (getenv("KC_DEBUG"))
		printf("%s(): skipped over %d bytes\n", __func__, BIO_tell(bio_chain));


	/* Read the database */
	if (getenv("KC_DEBUG"))
		printf("%s(): reading database\n", __func__);

	pos = 0;
	do {
		/* if we've reached the size of 'buf', grow it */
		if (buf_size <= pos) {
			buf_size += read_size;
			*buf = realloc(*buf, buf_size); malloc_check(*buf);
			if (getenv("KC_DEBUG"))
				printf("%s(): buf_size: %d\n", __func__, buf_size);
		}

		ret = BIO_read(bio_chain, *buf + pos, buf_size - pos);
		if (getenv("KC_DEBUG"))
			printf("%s(): BIO_read(): %d\n", __func__, (unsigned int)ret);
		switch (ret) {
			case 0:
				if (BIO_should_retry(bio_chain)) {
					if (getenv("KC_DEBUG"))
						printf("%s(): read delay\n", __func__);

					sleep(1);
					continue;
				}
			break;
			case -1:
				if (BIO_should_retry(bio_chain)) {
					if (getenv("KC_DEBUG"))
						printf("%s(): read delay\n", __func__);

					sleep(1);
					continue;
				} else {
					if (getenv("KC_DEBUG"))
						printf("%s(): BIO_read() error (don't retry): %s\n", __func__, strerror(errno));

					dprintf(STDERR_FILENO, "ERROR: There was an error while trying to read the database!\n");
				}
			break;
			case -2:
				if (getenv("KC_DEBUG"))
					printf("%s(): unsupported operation: %s\n", __func__, strerror(errno));

				dprintf(STDERR_FILENO, "ERROR: There was an error while trying to read the database!\n");
			break;
			default:
				pos += ret;
				if (getenv("KC_DEBUG"))
					printf("%s(): pos: %d\n", __func__, pos);
			break;
		}
	} while (ret > 0);

	if (getenv("KC_DEBUG"))
		printf("%s(): read %d bytes\n", __func__, pos);


	return(pos);
} /* kc_db_reader() */


signed char kc_arg_parser(int largc, char **largv, const char *opts, db_parameters *db_params, extra_parameters *extra_params)
{
	char		*ssha_type = NULL, *ssha_comment = NULL;

#ifdef _HAVE_YUBIKEY
	unsigned long int	ykchalresp = 0;
	yk_array		*yk = NULL;
	yk_array		*yk_1st = NULL;

	char			*param = NULL;
#endif
	char		*inv = NULL;
	int		c = 0;


	if (getenv("KC_DEBUG"))
		printf("%s(): caller='%s' with opts='%s'\n", __func__, extra_params->caller, opts);

#ifdef __FreeBSD__
	optind = 1;
#else
	optind = 0;
#endif

	while ((c = getopt(largc, largv, opts)) != -1)
		switch (c) {
			case 'c':
				if (strncmp(extra_params->caller, "main", 4) == 0) {
					extra_params->keychain_start = optarg;
				} else if (strncmp(extra_params->caller, "export", 6) == 0) {
					free(extra_params->cname); extra_params->cname = NULL;
					extra_params->cname = BAD_CAST strdup(optarg);
					if (!extra_params->cname) {
						perror("ERROR: Could not duplicate the keychain name");
						return(-1);
					}
				}
			break;
			case 'C':
				if (strncmp(extra_params->caller, "main", 4) == 0) {
					extra_params->keychain_start = optarg;
					extra_params->keychain_start_name = 1;
				}
			break;
			case 'r':
				if (strncmp(extra_params->caller, "main", 4) == 0) {
					db_params->readonly = 1;
				}
			break;
			case 'p':
				if (strncmp(extra_params->caller, "main", 4) == 0) {
					db_params->pass_filename = optarg;
					printf("Using password file: %s\n", db_params->pass_filename);
				}
			break;
			case 'b':
				if (strncmp(extra_params->caller, "main", 4) == 0)
					batchmode = 1;
			break;
			case 'B':
				if (strncmp(extra_params->caller, "main", 4) == 0)
					batchmode = 2;
			break;
			case 'A':
				if (strlen(db_params->ssha_type)  ||  strlen(db_params->ssha_comment)) {
					dprintf(STDERR_FILENO, "ERROR: Please specify the '-%c' option only once!\n", c);
					return(-1);
				}


				ssha_type = strndup(strsep(&optarg, ","), 11);
				if (ssha_type == NULL  ||  !strlen(ssha_type)) {
					dprintf(STDERR_FILENO, "ERROR: SSH key type is empty!\n");
					return(-1);
				}
				if (	strncmp(ssha_type, "ssh-rsa", 7) != 0  &&
					strncmp(ssha_type, "ssh-ed25519", 11) != 0
				) {
					dprintf(STDERR_FILENO, "ERROR: SSH key type is unsupported: '%s'\n", ssha_type);
					return(-1);
				}

				ssha_comment = strndup(strsep(&optarg, ","), 512);
				if (ssha_comment == NULL  ||  !strlen(ssha_comment)) {
					dprintf(STDERR_FILENO, "ERROR: SSH key comment is empty!\n");
					return(-1);
				}

				if (strlcpy(db_params->ssha_type, ssha_type, sizeof(db_params->ssha_type)) >= sizeof(db_params->ssha_type)) {
					dprintf(STDERR_FILENO, "ERROR: Error while getting SSH key type.\n");
					return(-1);
				}

				if (strlcpy(db_params->ssha_comment, ssha_comment, sizeof(db_params->ssha_comment)) >= sizeof(db_params->ssha_comment)) {
					dprintf(STDERR_FILENO, "ERROR: Error while getting SSH key comment.\n");
					return(-1);
				}

				if (optarg  &&  strncmp(optarg, "password", 8) == 0) {
					db_params->ssha_password = 1;
				}
			break;
			case 'k':
				if (strncmp(extra_params->caller, "main", 4) == 0) {
					db_params->db_filename = optarg;
				} else if (strncmp(extra_params->caller, "export", 6) == 0  ||
					strncmp(extra_params->caller, "import", 6) == 0
				) {
					free(db_params->db_filename); db_params->db_filename = NULL;
					db_params->db_filename = strdup(optarg);
					if (!db_params->db_filename) {
						perror("ERROR: Could not duplicate the database file name");
						return(-1);
					}
				}
			break;
			case 'P':
				if (db_params->kdf) {
					dprintf(STDERR_FILENO, "ERROR: Please specify the '-%c' option only once!\n", c);
					return(-1);
				}
				db_params->kdf = strdup(optarg); malloc_check(db_params->kdf);
			break;
			case 'K':
				if (db_params->key_len) {
					dprintf(STDERR_FILENO, "ERROR: Please specify the '-%c' option only once!\n", c);
					return(-1);
				}


				if (optarg[0] == '-') {
					dprintf(STDERR_FILENO, "ERROR: Key length parameter seems to be negative.\n");
					return(-1);
				}

				db_params->key_len = strtoul(optarg, &inv, 10);
				if (inv[0] != '\0') {
					dprintf(STDERR_FILENO, "ERROR: Unable to convert the key length parameter.\n");
					return(-1);
				}

				if (db_params->key_len < KEY_MIN_LEN  ||  db_params->key_len > KEY_MAX_LEN) {
					dprintf(STDERR_FILENO, "ERROR: Key length is outside the minimum (%d) and maximum (%d) limits!\n", KEY_MIN_LEN, KEY_MAX_LEN);
					return(-1);
				}
			break;
			case 'R':
				if (db_params->kdf_reps) {
					dprintf(STDERR_FILENO, "ERROR: Please specify the '-%c' option only once!\n", c);
					return(-1);
				}


				if (optarg[0] == '-') {
					dprintf(STDERR_FILENO, "ERROR: KDF iterations parameter seems to be negative.\n");
					return(-1);
				}

				db_params->kdf_reps = strtoul(optarg, &inv, 10);
				if (inv[0] != '\0') {
					dprintf(STDERR_FILENO, "ERROR: Unable to convert the KDF iterations parameter.\n");
					return(-1);
				}
			break;
			case 'e':
				if (db_params->cipher) {
					dprintf(STDERR_FILENO, "ERROR: Please specify the '-%c' option only once!\n", c);
					return(-1);
				}
				db_params->cipher = strdup(optarg); malloc_check(db_params->cipher);
			break;
			case 'm':
				if (db_params->cipher_mode) {
					dprintf(STDERR_FILENO, "ERROR: Please specify the '-%c' option only once!\n", c);
					return(-1);
				}
				db_params->cipher_mode = strdup(optarg); malloc_check(db_params->cipher_mode);
			break;
#ifdef _HAVE_YUBIKEY
			case 'Y':
				if (optarg[0] == '-') {
					dprintf(STDERR_FILENO, "ERROR: YubiKey slot/device parameter seems to be negative.\n");
					return(-1);
				}

				yk = malloc(sizeof(yk_array)); malloc_check(yk);
				yk->slot = 0;
				yk->dev = 0;
				yk->serial = 0;
				yk->next = NULL;

				/* slot number */
				ykchalresp = strtoul(strsep(&optarg, ","), &inv, 10);
				if (inv[0] == '\0') {
					if (ykchalresp > 2  ||  ykchalresp < 1) {
						dprintf(STDERR_FILENO, "ERROR: YubiKey slot number is not 1 or 2.\n");
						return(-1);
					}

					yk->slot = ykchalresp;
				} else {
					dprintf(STDERR_FILENO, "ERROR: Unable to convert the YubiKey slot parameter.\n");
					return(-1);
				}

				/* device index or serial number */
				if (optarg) {
					param = strsep(&optarg, ",");
					ykchalresp = strtoul(param, &inv, 10);
					if (inv[0] == '\0') {
						if (ykchalresp < 0) {
							dprintf(STDERR_FILENO, "ERROR: YubiKey device parameter is invalid.\n");
							return(-1);
						}

						/* I guess this'll be an arbitrary limit on
						 * how many YubiKeys can be used -> 10 */
						if (ykchalresp < 10)
							yk->dev = ykchalresp;
						else
							yk->serial = ykchalresp;
					}
					/* no error handling here, this can
					 * still be the ',password' parameter,
					 * in which case we default to device #0
					 */
				}

				/* ',password' parameter, either the remnant of
				 * the previous 'device' parameter parsing or
				 * actually the third parameter in
				 * -Yslot,device,password */
				if (param  &&  strncmp(param, "password", 8) == 0)
					db_params->yk_password++;
				else if (optarg  &&  strncmp(strsep(&optarg, ","), "password", 8) == 0)
					db_params->yk_password++;


				if (!(db_params->yk)) {
					db_params->yk = yk;
				} else {
					yk_1st = db_params->yk;

					while (db_params->yk) {
						if (!(db_params->yk->next))
							break;
						db_params->yk = db_params->yk->next;
					}

					db_params->yk->next = yk;
					db_params->yk = yk_1st;
				}
			break;
#endif
			case '1':
				db_params->first = strdup(optarg); malloc_check(db_params->first);
			break;
			case '2':
				db_params->second = strdup(optarg); malloc_check(db_params->second);
			break;
			case '3':
				db_params->third = strdup(optarg); malloc_check(db_params->third);
			break;
			case '4':
				db_params->fourth = strdup(optarg); malloc_check(db_params->fourth);
			break;
			case '5':
				db_params->fifth = strdup(optarg); malloc_check(db_params->fifth);
			break;
			case 'o':
				if (strncmp(extra_params->caller, "import", 6) == 0)
					extra_params->legacy++;
			break;
			case 'v':
				if (strncmp(extra_params->caller, "main", 4) == 0)
					return(2);
			break;
			case 'h':
			default:
				return(0);
			break;
		}

	/* clean up after option parsing */
	free(ssha_type); ssha_type = NULL;
	free(ssha_comment); ssha_comment = NULL;

	/* print some status information after parsing the options */
	if (	(strlen(db_params->ssha_type)  &&  db_params->yk)  &&
		(!db_params->ssha_password  ||  !db_params->yk_password)
	) {
		dprintf(STDERR_FILENO, "ERROR: Using -A and -Y together only makes sense with the ',password' parameter for both of them!\n");
		return(-1);
	}


	return(1);
} /* kc_arg_parser() */
