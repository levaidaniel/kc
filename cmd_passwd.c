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
extern char		*cipher_mode;
extern int		db_file;

extern unsigned char	salt[17], iv[17], key[128];


void
cmd_passwd(const char *e_line, command *commands)
{
	char		*pass = NULL;
	char		ret = -1;


	/* ask for the new password */
	while (ret == -1)
		ret = password_read(&pass);

	if (ret == 0)	/* canceled */
		return;

	kc_gen_crypt_params(KC_GENERATE_IV | KC_GENERATE_SALT | KC_GENERATE_KEY, pass);

	if (pass)
		memset(pass, '\0', PASSWORD_MAXLEN);
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

	cmd_write(NULL, NULL);
} /* cmd_passwd() */
