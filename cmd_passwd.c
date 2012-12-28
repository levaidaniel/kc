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

#include "common.h"
#include "commands.h"

#ifndef _LINUX
#include <fcntl.h>
#include <readpassphrase.h>
#else
#include <sys/file.h>
#include <bsd/readpassphrase.h>
#endif


extern BIO		*bio_cipher;
extern size_t		pass_maxlen;
extern int		db_file;
extern char		*cipher_mode;


void
cmd_passwd(const char *e_line, command *commands)
{
	unsigned char	key[128];
	char		*pass = NULL, *rand_str = NULL;
	unsigned char	salt[17], iv[17];


	/* ask for the new password */
	pass = malloc(pass_maxlen + 1); malloc_check(pass);
	readpassphrase("New password: ", pass, pass_maxlen + 1, RPP_ECHO_OFF | RPP_REQUIRE_TTY);


	/* regenerate the IV and the salt. */
	if (getenv("KC_DEBUG"))
		puts("regenerating salt and IV");

	rand_str = get_random_str(sizeof(iv) - 1, 0);
	if (!rand_str) {
		puts("IV generation failure!");
		return;
	}
	strlcpy((char *)iv, rand_str, sizeof(iv));
	free(rand_str);

	rand_str = get_random_str(sizeof(salt) - 1, 0);
	if (!rand_str) {
		puts("Salt generation failure!");
		return;
	}
	strlcpy((char *)salt, rand_str, sizeof(salt));
	free(rand_str);

	if (getenv("KC_DEBUG"))
		printf("iv='%s'\nsalt='%s'\n", iv, salt);

	/* regenerate the key for encoding with the new salt value */
	PKCS5_PBKDF2_HMAC_SHA1(pass, (int)strlen(pass), salt, sizeof(salt), 5000, 128, key);

	memset(pass, '\0', pass_maxlen);
	free(pass); pass = NULL;


	/* reconfigure encoding with the newly generated key and IV */
	if (strcmp(cipher_mode, "cfb128") == 0) {
		if (getenv("KC_DEBUG"))
			printf("using cipher mode: %s\n", cipher_mode);
		BIO_set_cipher(bio_cipher, EVP_aes_256_cfb128(), key, iv, 1);
	} else if (strcmp(cipher_mode, "ofb") == 0) {
		if (getenv("KC_DEBUG"))
			printf("using cipher mode: %s\n", cipher_mode);
		BIO_set_cipher(bio_cipher, EVP_aes_256_ofb(), key, iv, 1);
	} else {	/* the default is CBC */
		if (getenv("KC_DEBUG"))
			printf("using default cipher mode: %s\n", cipher_mode);
		BIO_set_cipher(bio_cipher, EVP_aes_256_cbc(), key, iv, 1);
	}


	/* save the database with the new encryption parameters */
	if (ftruncate(db_file, 0) != 0) {
		puts("There was an error while trying to save the XML document!");
		if (getenv("KC_DEBUG"))
			perror("db file truncate");

		return;
	}
	lseek(db_file, 0, SEEK_SET);

	write(db_file, iv, sizeof(iv) - 1);
	write(db_file, salt, sizeof(salt) - 1);

	cmd_write(NULL, NULL);
} /* cmd_passwd() */
