/*
 * Copyright (c) 2011-2013 LEVAI Daniel
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

#ifdef BSD
#include <fcntl.h>
#include <readpassphrase.h>
#endif

#ifdef __OpenBSD__
#include <util.h>
#endif

#ifdef __linux__
#include <sys/file.h>
#include <bsd/readpassphrase.h>
#include <stdint.h>
#include "bcrypt/bcrypt_pbkdf.h"
#endif

#ifdef _HAVE_LIBSCRYPT
#include <libscrypt.h>
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
	int		i = 0;
	int		rnd_file = -1;
#ifdef __linux__
	char		*rnd_dev = "/dev/urandom";
#else
	char		*rnd_dev = "/dev/random";
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
						perror("read(random device)");

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
						perror("read(random device)");

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
					perror("read(random device)");

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

		rnd_str[i] = *tmp;              /* store the value */
		*tmp = '\0';            /* reset the value */
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


	if (!line)
		return(xmlStrdup(BAD_CAST ""));


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
					puts("Random number generation failure!");

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
get_line(const xmlChar *value_nl, const long int idx)
{
	xmlChar	*line = NULL;

	long int	nl = 1, pos = 0, tmp = 0, length = 0;


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


	if (!line)
		return(xmlStrdup(BAD_CAST ""));

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


char
kc_password_read(char **pass1, const unsigned char new)
{
	/*
	 * returns:
	 * -1 mismatch
	 *  0 cancel
	 *  1 ok
	 */

	char	*pass2 = NULL;
	int	rpp_flags = 0;


	rpp_flags = RPP_ECHO_OFF;
	if (batchmode)
		rpp_flags |= RPP_STDIN;
	else
		rpp_flags |= RPP_REQUIRE_TTY;


	*pass1 = malloc(PASSWORD_MAXLEN + 1); malloc_check(*pass1);

	if (new)
		readpassphrase("New password (empty to cancel): ", *pass1, PASSWORD_MAXLEN + 1, rpp_flags);
	else
		readpassphrase("Password: ", *pass1, PASSWORD_MAXLEN + 1, rpp_flags);

	if (new) {
		if (!strlen(*pass1)) {
			if (*pass1)
				memset(*pass1, '\0', PASSWORD_MAXLEN);
			free(*pass1); *pass1 = NULL;

			puts("Canceled.");
			return(0);
		}

		pass2 = malloc(PASSWORD_MAXLEN + 1); malloc_check(pass2);
		readpassphrase("New password again (empty to cancel): ", pass2, PASSWORD_MAXLEN + 1, rpp_flags);
		if (!strlen(pass2)) {
			if (*pass1)
				memset(*pass1, '\0', PASSWORD_MAXLEN);
			free(*pass1); *pass1 = NULL;

			if (pass2)
				memset(pass2, '\0', PASSWORD_MAXLEN);
			free(pass2); pass2 = NULL;

			puts("Canceled.");
			return(0);
		}

		if (strcmp(*pass1, pass2) != 0) {
			if (*pass1)
				memset(*pass1, '\0', PASSWORD_MAXLEN);
			free(*pass1); *pass1 = NULL;

			if (pass2)
				memset(pass2, '\0', PASSWORD_MAXLEN);
			free(pass2); pass2 = NULL;

			puts("Passwords mismatch.");
			return(-1);
		}

		if (pass2)
			memset(pass2, '\0', PASSWORD_MAXLEN);
		free(pass2); pass2 = NULL;
	}

	return(1);
} /* kc_password_read() */


char
kc_setup_crypt(BIO *bio_chain, const unsigned int enc, struct db_parameters *db_params, const unsigned int flags)
{
	char	*iv_tmp = NULL, *salt_tmp = NULL;
	int	i = 0;

	EVP_MD_CTX	*mdctx = NULL;
	unsigned char	digested[EVP_MAX_MD_SIZE];
	unsigned int	digested_len = 0;
	char		hex_tmp[3];


	if ((flags & KC_SETUP_CRYPT_IV)) {
		iv_tmp = get_random_str(IV_LEN, 2);
		if (!iv_tmp) {
			puts("IV generation failure!");

			return(0);
		}
	}

	if ((flags & KC_SETUP_CRYPT_SALT)) {
		salt_tmp = get_random_str(SALT_LEN, 2);
		if (!salt_tmp) {
			puts("Salt generation failure!");

			free(iv_tmp); iv_tmp = NULL;
			return(0);
		}
	}


	if ((flags & KC_SETUP_CRYPT_IV)) {
		/* Setup the digest context */
		mdctx = EVP_MD_CTX_create();
		if (!mdctx) {
			puts("Could not create digest context for IV!");

			free(iv_tmp); iv_tmp = NULL;
			free(salt_tmp); salt_tmp = NULL;
			return(0);
		}
		EVP_DigestInit_ex(mdctx, EVP_sha512(), NULL);
		EVP_DigestUpdate(mdctx, iv_tmp, IV_LEN);
		EVP_DigestFinal(mdctx, digested, &digested_len);

		/* Print the binary digest in hex as characters (numbers, effectively)
		 * into the ..->iv variable.
		 */
		for (i = 0; i < digested_len; i++) {
			snprintf(hex_tmp, 3, "%02x", digested[i]);

			if (!i)
				strlcpy((char *)db_params->iv, hex_tmp, IV_DIGEST_LEN + 1);
			else
				strlcat((char *)db_params->iv, hex_tmp, IV_DIGEST_LEN + 1);
		}

		free(iv_tmp); iv_tmp = NULL;
	}
	if (getenv("KC_DEBUG"))
		printf("iv='%s'\n", db_params->iv);


	if ((flags & KC_SETUP_CRYPT_SALT)) {
		/* Setup the digest context */
		mdctx = EVP_MD_CTX_create();
		if (!mdctx) {
			puts("Could not create digest context for IV!");

			free(iv_tmp); iv_tmp = NULL;
			free(salt_tmp); salt_tmp = NULL;
			return(0);
		}
		EVP_DigestInit_ex(mdctx, EVP_sha512(), NULL);
		EVP_DigestUpdate(mdctx, salt_tmp, SALT_LEN);
		EVP_DigestFinal(mdctx, digested, &digested_len);

		/* Print the binary digest in hex as characters (numbers, effectively)
		 * into the ..->salt variable.
		 */
		for (i = 0; i < digested_len; i++) {
			snprintf(hex_tmp, 3, "%02x", digested[i]);

			if (!i)
				strlcpy((char *)db_params->salt, hex_tmp, SALT_DIGEST_LEN + 1);
			else
				strlcat((char *)db_params->salt, hex_tmp, SALT_DIGEST_LEN + 1);
		}

		free(salt_tmp); salt_tmp = NULL;
	}
	if (getenv("KC_DEBUG"))
		printf("salt='%s'\n", db_params->salt);


	if ((flags & KC_SETUP_CRYPT_KEY)) {
		if (getenv("KC_DEBUG"))
			printf("generating new key from pass and salt.\n");

		/* Generate a proper key from the user's password */
		if (strcmp(db_params->kdf, "sha1") == 0)
			PKCS5_PBKDF2_HMAC_SHA1(db_params->pass, (int)strlen(db_params->pass), db_params->salt, SALT_DIGEST_LEN + 1, 5000, KEY_LEN, db_params->key);
		else if (strcmp(db_params->kdf, "sha512") == 0)
			PKCS5_PBKDF2_HMAC(db_params->pass, (int)strlen(db_params->pass), db_params->salt, SALT_DIGEST_LEN + 1, 5000, EVP_sha512(), KEY_LEN, db_params->key);
#ifdef _HAVE_BCRYPT
		else if (strcmp(db_params->kdf, "bcrypt") == 0)
			bcrypt_pbkdf(db_params->pass, strlen(db_params->pass), db_params->salt, SALT_DIGEST_LEN + 1, db_params->key, KEY_LEN, 16);
#endif
#ifdef _HAVE_LIBSCRYPT
		else if (strcmp(db_params->kdf, "scrypt") == 0)
			libscrypt_scrypt((const unsigned char *)db_params->pass, strlen(db_params->pass), db_params->salt, SALT_DIGEST_LEN + 1, SCRYPT_N, SCRYPT_r, SCRYPT_p, db_params->key, KEY_LEN);
#endif
		else {
			printf("Unknown kdf: %s!\n", db_params->kdf);
			return(0);
		}
	}

	if (getenv("KC_DEBUG"))
		printf("crypt setup: using %s based KDF\n", db_params->kdf);

	if (!db_params->key) {
		puts("Key generation failure!");

		return(0);
	}


	/* extract bio_cipher from bio_chain */
	bio_chain = BIO_find_type(bio_chain, BIO_TYPE_CIPHER);
	if (!bio_chain) {
		puts("Could not find a usable cipher method!");

		return(0);
	}


	/* reconfigure encoding with the key and IV */
	if (strcmp(db_params->cipher_mode, "cfb128") == 0)
		BIO_set_cipher(bio_chain, EVP_aes_256_cfb128(), db_params->key, db_params->iv, enc);
	else if (strcmp(db_params->cipher_mode, "ofb") == 0)
		BIO_set_cipher(bio_chain, EVP_aes_256_ofb(), db_params->key, db_params->iv, enc);
	else if (strcmp(db_params->cipher_mode, "cbc") == 0)
		BIO_set_cipher(bio_chain, EVP_aes_256_cbc(), db_params->key, db_params->iv, enc);
	else {
		printf("Unknown cipher mode: %s!\n", db_params->cipher_mode);
		return(0);
	}

	if (getenv("KC_DEBUG"))
		printf("crypt setup: using %s cipher mode\n", db_params->cipher_mode);


	return(1);
} /* kc_setup_crypt() */


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
		perror("BIO_new_file()");
		return(NULL);
	}
	BIO_set_close(bio_file, BIO_CLOSE);	/* On ignoring the return value: BIO_set_close() always returns 1, according to the openssl documentation. */
	bio_chain = BIO_push(bio_file, bio_chain);

	bio_b64 = BIO_new(BIO_f_base64());
	if (!bio_b64) {
		perror("BIO_new(f_base64)");
		return(NULL);
	}
	bio_chain = BIO_push(bio_b64, bio_chain);

	bio_cipher = BIO_new(BIO_f_cipher());
	if (!bio_cipher) {
		perror("BIO_new(f_cipher)");
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
			printf("xml_buf content:\n'%s'(%d)\n", xmlBufferContent(xml_buf), (int)xmlBufferLength(xml_buf));
		xmlSaveClose(xml_save);


		/* write the IV */
		if (getenv("KC_DEBUG"))
			puts("writing IV");

		remaining = IV_DIGEST_LEN;
		while (remaining > 0) {
			ret = write(db_params->db_file, db_params->iv, remaining);

			if (ret < 0) {
				perror("writing IV to database file");
				return(0);
			}

			remaining -= ret;

			if (getenv("KC_DEBUG")) {
				printf("wrote: %d\n", ret);
				printf("remaining: %d\n", remaining);
			}
		}
		/* Write a newline at the end */
		ret = write(db_params->db_file, "\n", 1);
		if (ret != 1) {
			perror("writing IV to database file");
			return(0);
		}


		/* write the salt */
		if (getenv("KC_DEBUG"))
			puts("writing salt");

		remaining = SALT_DIGEST_LEN;
		while (remaining > 0) {
			ret = write(db_params->db_file, db_params->salt, remaining);

			if (ret < 0) {
				perror("writing salt to database file");
				return(0);
			}

			remaining -= ret;

			if (getenv("KC_DEBUG")) {
				printf("wrote: %d\n", ret);
				printf("remaining: %d\n", remaining);
			}
		}
		/* Write a newline at the end */
		ret = write(db_params->db_file, "\n", 1);
		if (ret != 1) {
			perror("writing salt to database file");
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
			puts("writing database");

		remaining = xmlBufferLength(xml_buf);
		while (remaining > 0) {
			ret = BIO_write(bio_chain, xmlBufferContent(xml_buf), remaining);

			if (ret <= 0) {
				if (BIO_should_retry(bio_chain)) {
					if (getenv("KC_DEBUG"))
						puts("write delay");

					sleep(1);
					continue;
				} else {
					if (getenv("KC_DEBUG"))
						puts("BIO_write() error (don't retry)");

					break;
				}
			}

			remaining -= ret;

			if (getenv("KC_DEBUG")) {
				printf("wrote: %d\n", ret);
				printf("remaining: %d\n", remaining);
			}
		}


		/* Flush the written stuff */
		do {
			if (BIO_flush(bio_chain) == 1) {
				if (getenv("KC_DEBUG"))
					puts("flushed bio_chain");
			} else {
				if (BIO_should_retry(bio_chain)) {
					if (getenv("KC_DEBUG"))
						puts("flush delay");

					sleep(1);
					continue;
				} else {
					if (getenv("KC_DEBUG"))
						puts("BIO_should_retry() is false");

					break;
				}
			}
		} while(BIO_wpending(bio_chain) > 0);

		if (getenv("KC_DEBUG"))
			printf("db_file size -> %d\n", BIO_tell(bio_chain));

		xmlBufferFree(xml_buf);

		return(1);
	} else {
		if (getenv("KC_DEBUG"))
			puts("xmlSaveToBuffer() error");

		return(0);
	}
} /* kc_db_writer() */


char
kc_validate_xml(xmlDocPtr db)
{
	xmlParserInputBufferPtr	buf = NULL;
	xmlDtdPtr		dtd = NULL;
	xmlValidCtxtPtr		valid_ctx = NULL;


	buf = xmlParserInputBufferCreateMem(KC_DTD, sizeof(KC_DTD), XML_CHAR_ENCODING_NONE);
	if (!buf) {
		if (getenv("KC_DEBUG"))
			xmlGenericError(xmlGenericErrorContext, "Could not allocate buffer for DTD.\n");

		return(0);
	}

	/* XXX xmlIOParseDTD() is supposed to free the ParserInputBuffer context.
	 * https://mail.gnome.org/archives/xml/2001-July/msg00035.html
	 */
	dtd = xmlIOParseDTD(NULL, buf, XML_CHAR_ENCODING_NONE);
	if (!dtd) {
		if (getenv("KC_DEBUG"))
			xmlGenericError(xmlGenericErrorContext, "Could not parse kc DTD.\n");

		xmlFreeParserInputBuffer(buf);
		return(0);
	}

	valid_ctx = xmlNewValidCtxt();
	if (!valid_ctx ) {
		if (getenv("KC_DEBUG"))
			xmlGenericError(xmlGenericErrorContext, "Could not allocate a new validation context.\n");

		return(0);
	}


	if (!xmlValidateDtd(valid_ctx, db, dtd)) {
		if (getenv("KC_DEBUG"))
			xmlGenericError(xmlGenericErrorContext, "Validation failed against kc DTD.\n");

		xmlFreeValidCtxt(valid_ctx);
		return(0);
	}


	xmlFreeValidCtxt(valid_ctx);


	return(1);
} /* kc_validate_xml() */


int
kc_db_reader(char **buf, BIO *bio_chain)
{
	int		pos = 0, buf_size = 1024;
	ssize_t		ret = -1;


	/* Seek after the IV and salt */
	if (getenv("KC_DEBUG"))
		puts("skipping over IV and salt while reading db");

	BIO_seek(bio_chain, IV_DIGEST_LEN + SALT_DIGEST_LEN + 2);

	if (getenv("KC_DEBUG"))
		printf("skipped over %d bytes\n", BIO_tell(bio_chain));


	/* Read the database */
	*buf = calloc(1, buf_size); malloc_check(*buf);

	if (getenv("KC_DEBUG"))
		puts("reading database");

	pos = 0;
	do {
		/* if we've reached the size of 'buf', grow it */
		if (buf_size <= pos) {
			buf_size += 1024;
			*buf = realloc(*buf, buf_size); malloc_check(*buf);
		}

		ret = BIO_read(bio_chain, *buf + pos, buf_size - pos);
		if (getenv("KC_DEBUG"))
			printf("BIO_read(): %d\n", (unsigned int)ret);
		switch (ret) {
			case 0:
				if (BIO_should_retry(bio_chain)) {
					if (getenv("KC_DEBUG"))
						puts("read delay");

					sleep(1);
					continue;
				}
			break;
			case -1:
				if (BIO_should_retry(bio_chain)) {
					if (getenv("KC_DEBUG"))
						puts("read delay");

					sleep(1);
					continue;
				} else {
					if (getenv("KC_DEBUG"))
						perror("BIO_read() error (don't retry)");

					puts("There was an error while trying to read the database!");
				}
			break;
			case -2:
				if (getenv("KC_DEBUG"))
					perror("unsupported operation");

				puts("There was an error while trying to read the database!");
			break;
			default:
				pos += ret;
				if (getenv("KC_DEBUG"))
					printf("pos: %d\n", pos);
			break;
		}
	} while (ret > 0);

	if (getenv("KC_DEBUG"))
		printf("read %d bytes\n", pos);


	return(pos);
} /* kc_db_reader() */
