/*
 * Copyright (c) 2018-2023 LEVAI Daniel
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

/*
 * Bits and pieces stolen from OpenBSD's OpenSSH repository, namely usr.bin/ssh/authfd.[ch]
 */
/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * Functions for connecting the local authentication agent.
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 *
 * SSH2 implementation,
 * Copyright (c) 2000 Markus Friedl.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "common.h"
#include "ssha.h"


void
put_u32(void *vp, u_int32_t v)
{
	u_char *p = (u_char *)vp;

	p[0] = (u_char)(v >> 24) & 0xff;
	p[1] = (u_char)(v >> 16) & 0xff;
	p[2] = (u_char)(v >> 8) & 0xff;
	p[3] = (u_char)v & 0xff;
}


u_int32_t
get_u32(const void *vp)
{
        const u_char *p = (const u_char *)vp;
        u_int32_t v;

        v  = (u_int32_t)p[0] << 24;
        v |= (u_int32_t)p[1] << 16;
        v |= (u_int32_t)p[2] << 8;
        v |= (u_int32_t)p[3];

        return (v);
}


int
kc_ssha_connect(void)
{
	int	sock = -1;
	struct	sockaddr_un addr;
	char	*sock_name = NULL;


	memset(&addr, '\0', sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;

	sock_name = getenv("SSH_AUTH_SOCK");
	if (getenv("KC_DEBUG"))
		printf("%s(): SSH agent UNIX socket: '%s'\n", __func__, sock_name);
	if (sock_name == NULL  ||  strlen(sock_name) <= 0) {
		dprintf(STDERR_FILENO, "ERROR: SSH agent socket name is NULL or zero length\n");
		return(-1);
	}

	if (strlcpy(addr.sun_path, sock_name, sizeof(addr.sun_path)) >= sizeof(addr.sun_path)) {
		dprintf(STDERR_FILENO, "ERROR: Truncated strlcpy() when copying sock_name to sun_path\n");
		return(-1);
	}

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock <= 0) {
		dprintf(STDERR_FILENO, "ERROR: Could not create UNIX socket for SSH agent communication\n");
		return(-1);
	}

	if (connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) != 0) {
		dprintf(STDERR_FILENO, "ERROR: Could not connect to SSH agent UNIX socket: '%s'\n", sock_name);
		return(-1);
	}

	return(sock);
} /* kc_ssha_connect() */


char
kc_ssha_identity_list_request(int sock)
{
	char	*buf = NULL;
	char	request = SSH2_AGENTC_REQUEST_IDENTITIES;
	size_t	w = 0, buf_len = 1024;


	buf = calloc(1, buf_len); malloc_check(buf);

	/* my request's length */
	put_u32(buf, sizeof(request));
	w = write(sock, buf, 4);
	if (w != 4 ) {
		perror("ERROR: socket write() error");
		free(buf); buf = NULL;
		return(-1);
	} else {
		/* reset the send buffer */
		memset(buf, '\0', buf_len);
	}

	/* my request */
	memcpy(buf, &request, 1);
	w = write(sock, buf, 1);
	if (w != 1 ) {
		perror("ERROR: socket write() error");
		free(buf); buf = NULL;
		return(-1);
	}


	free(buf); buf = NULL;
	return(1);
} /* kc_ssha_identity_list_request() */


char
kc_ssha_sign_request(int sock, struct kc_ssha_identity *id, char *data, unsigned int data_len, unsigned int flags)
{
	char	*buf = NULL;
	char	len[4];
	char	request = SSH2_AGENTC_SIGN_REQUEST;
	size_t	w = 0, wt = 0, buf_len = 0, pos = 0, req_len = 0;


	if (data == NULL) {
		dprintf(STDERR_FILENO, "ERROR: Data to sign is empty\n");
		return(-1);
	}

	/* 5 is the request's length field (without the flags field), plus
	 * request that is one byte
	 * 4 is the flags field */
	req_len = 5 + 4 + id->pubkey_len + 4 + data_len;
	buf_len = req_len + 4;	/* plus flags field */

	buf = calloc(1, buf_len); malloc_check(buf);

	/* my request's length */
	put_u32(len, req_len);
	memcpy(buf+pos, len, 4);
	memset(len, '\0', 4);
	pos += 4;

	/* my request */
	memcpy(buf+pos, &request, 1);
	pos += 1;

	/* my key's length */
	put_u32(len, id->pubkey_len);
	memcpy(buf+pos, len, 4);
	memset(len, '\0', 4);
	pos += 4;

	/* pubkey */
	memcpy(buf+pos, id->pubkey, id->pubkey_len);
	pos += id->pubkey_len;

	/* my data's length */
	put_u32(len, data_len);
	memcpy(buf+pos, len, 4);
	memset(len, '\0', 4);
	pos += 4;

	/* data to sign */
	memcpy(buf+pos, data, data_len);
	pos += data_len;

	/* flags */
	/* This is not 'len' per se, I'm just using this variable. This is the
	 * flags parameter which is an uint32 */
	put_u32(len, flags);
	memcpy(buf+pos, len, 4);
	pos += 4;

	if (buf_len != pos) {
		dprintf(STDERR_FILENO, "ERROR: Total copied bytes is different from the buffer size\n");
		free(buf); buf = NULL;
		return(-1);
	}

	pos = 0;
	do {
		w = write(sock, buf+pos, buf_len-pos);
		if (w <= 0) {
			perror("ERROR: socket write() error");
			free(buf); buf = NULL;
			return(-1);
		}

		pos += w;
		wt += w;
	} while (pos < buf_len);

	if (getenv("KC_DEBUG"))
		printf("%s(): wrote total = %zd\n", __func__, wt);


	free(buf); buf = NULL;
	return(1);
} /* kc_ssha_sign_request() */


struct kc_ssha_response*
kc_ssha_get_full_response(int sock)
{
	char	*buf = NULL, *nbuf = NULL;
	size_t	buf_len_base = MAX_AGENT_REPLY_LEN/256, buf_len_mult = 1;
	size_t	buf_len = buf_len_mult * buf_len_base, buf_len_prev = 0;
	size_t	buf_rem = buf_len, r = 0, rt = 0;
	size_t	len = 0, rem = 0;

	struct kc_ssha_response	*response = NULL;


	buf = malloc(buf_len); malloc_check(buf);

	/* first read the response's length */
	r = read(sock, buf, 4);
	if (r != 4 ) {
		perror("ERROR: socket read() error");
		free(buf); buf = NULL;
		return(NULL);
	};

	len = get_u32(buf);
	if (getenv("KC_DEBUG"))
		printf("%s(): response length = %zd\n", __func__, len);

	if (len > MAX_AGENT_REPLY_LEN) {
		dprintf(STDERR_FILENO, "ERROR: Indicated response length(%zd) is more than MAX_AGENT_REPLY_LEN(%d)\n", len, MAX_AGENT_REPLY_LEN);
		free(buf); buf = NULL;
		return(NULL);
	} else {
		/* reset the send buffer */
		memset(buf, '\0', buf_len);
	}

	rem = len;
	while (rem  &&  buf_rem  &&  buf_len < MAX_AGENT_REPLY_LEN) {
		if (rem > buf_rem) {
			buf_len_prev = buf_len;
			buf_len = ++buf_len_mult * buf_len_base;
			if (getenv("KC_DEBUG"))
				printf("%s(): realloc response buf to %zd\n", __func__, buf_len);
			nbuf = realloc(buf, buf_len); malloc_check(nbuf);

			buf = nbuf;
			buf_rem += buf_len - buf_len_prev;

			continue;
		}

		r = read(sock, buf+(buf_len - buf_rem), rem);
		switch (r) {
			case -1:
				perror("ERROR: socket read() error");
				free(buf); buf = NULL;
				return(NULL);
			break;
			case 0:
			default:
				rem -= r;	/* this is what remained in the socket */
				rt += r;	/* this is total bytes read */
				buf_rem -= r;	/* this is what my buffer can still hold */
			break;
		};
	}

	if (getenv("KC_DEBUG"))
		printf("%s(): read total = %zd\n", __func__, rt);

	if (rt != len) {
		dprintf(STDERR_FILENO, "ERROR: Total read bytes is different from length indication in response\n");
		free(buf); buf = NULL;
		return(NULL);
	}


	/* create the response struct */
	response = malloc(sizeof(struct kc_ssha_response)); malloc_check(response);

	response->type = (char)*buf;
	if (getenv("KC_DEBUG"))
		printf("%s(): response type is %d\n", __func__, response->type);
	/* allocate memory for the response data without the response type */
	response->data = malloc(rt - 1); malloc_check(response->data);
	memcpy(response->data, buf + 1, rt - 1);
	response->length = rt - 1;


	free(buf); buf = NULL;


	return(response);
} /* kc_ssha_get_full_response() */


struct kc_ssha_identity*
kc_ssha_parse_identities(struct kc_ssha_response *response)
{
	struct kc_ssha_identity		*idlist = NULL, *idlist_first = NULL;
	size_t	pos = 0;
	size_t	num_ids = 0, slen = 0, i = 0;


	num_ids = get_u32(response->data+pos);
	if (getenv("KC_DEBUG"))
		printf("%s(): SSH agent has %zd identities\n", __func__, num_ids);

	if (num_ids <= 0) {
		dprintf(STDERR_FILENO, "ERROR: SSH agent has no identities\n");
		return(NULL);
	}
	pos += 4;

	for (i = 0; i < num_ids; i++) {
		if (!i) {	/* first call */
			idlist = malloc(sizeof(struct kc_ssha_identity)); malloc_check(idlist);
			idlist_first = idlist;
		} else {
			if (idlist) {
				idlist->next = malloc(sizeof(struct kc_ssha_identity)); malloc_check(idlist->next);
				idlist = idlist->next;
			} else {	/* last ID was erroneous, and we had free()'d 'idlist' */
				idlist = malloc(sizeof(struct kc_ssha_identity)); malloc_check(idlist);
			}
		}

		idlist->pubkey_len = 0;
		idlist->pubkey = NULL;
		idlist->type = NULL;
		idlist->comment = NULL;
		idlist->next = NULL;

		/* pubkey length */
		idlist->pubkey_len = get_u32(response->data+pos);
		if (idlist->pubkey_len > KC_MAX_PUBKEY_LEN) {
			dprintf(STDERR_FILENO, "ERROR: Public key length is larger than allowed(%d), skipping\n", KC_MAX_PUBKEY_LEN);

			free(idlist); idlist = NULL;
			if (!i)	/* first call */
				idlist_first = NULL;

			continue;
		}
		pos += 4;

		/* public key */
		idlist->pubkey = malloc(idlist->pubkey_len); malloc_check(idlist->pubkey);
		memcpy(idlist->pubkey, response->data+pos, idlist->pubkey_len);
		pos += idlist->pubkey_len;

		/* key type
		 * get the key-type string's length and extract the key type,
		 * which are part of the pubkey blob
		 */
		slen = get_u32(idlist->pubkey);
		idlist->type = malloc(slen + 1); malloc_check(idlist->type);
		memcpy(idlist->type, idlist->pubkey+4, slen); idlist->type[slen] = '\0';
		/*				^^^^^ + 4 here is the length indication of the key type string */

		/* key comment */
		slen = get_u32(response->data+pos);
		if (slen > KC_MAX_PUBKEY_COMMENT_LEN) {
			dprintf(STDERR_FILENO, "ERROR: Public key (%s) comment length is larger than allowed(%d), skipping\n", idlist->type, KC_MAX_PUBKEY_COMMENT_LEN);

			free(idlist->pubkey); idlist->pubkey = NULL;
			free(idlist->type); idlist->type = NULL;
			free(idlist); idlist = NULL;
			if (!i)	/* first call */
				idlist_first = NULL;

			continue;
		}
		pos += 4;
		idlist->comment = malloc(slen + 1); malloc_check(idlist->comment);
		memcpy(idlist->comment, response->data+pos, slen); idlist->comment[slen] = '\0';
		pos += slen;

		if (getenv("KC_DEBUG"))
			printf("%s(): %zd. parsed SSH ID: (%s) %s, %zd long\n", __func__, i, idlist->type, idlist->comment, idlist->pubkey_len);
	}


	return(idlist_first);
} /* kc_ssha_parse_identities() */


struct kc_ssha_signature *
kc_ssha_parse_signature(struct kc_ssha_response *response)
{
	size_t	pos = 0;
	size_t	len = 0;
	char	*s = NULL;
	struct kc_ssha_signature	*signature = NULL;

	len = get_u32(response->data+pos);
	pos += 4;
	if (getenv("KC_DEBUG"))
		printf("%s(): signature length is %zd\n", __func__, len);

	if (len > KC_MAX_SIGNATURE_LEN) {
		dprintf(STDERR_FILENO, "ERROR: Signature length is larger than allowed\n");
		return(NULL);
	}
	s = calloc(1, len); malloc_check(s);
	memcpy(s, response->data+pos, len);

	signature = calloc(1, sizeof(struct kc_ssha_signature)); malloc_check(signature);
	signature->signature = s;
	signature->length = len;

	return(signature);
} /* kc_ssha_parse_signature() */


int
kc_ssha_get_password(struct db_parameters *db_params)
{
	int		sock = -1;
	unsigned int	data_to_sign_len = 0;
	char		ret = 0;
	char		*data_to_sign = NULL, *passtmp = NULL;
	size_t		passtmp_len = 0;
	struct kc_ssha_response		*response = NULL;
	struct kc_ssha_identity		*idlist = NULL, *idlist_prev = NULL;
	struct kc_ssha_signature 	*signature = NULL;


	if (strlen(db_params->ssha_type))
		printf("Using (%s) %s identity%s\n", db_params->ssha_type, db_params->ssha_comment, (db_params->ssha_password ? " and a password" : ""));

	sock = kc_ssha_connect();
	if (sock < 0) {
		dprintf(STDERR_FILENO, "ERROR: Couldn't establish UNIX socket connection\n");
		goto exiting;
	}

	/* get identities */
	if (kc_ssha_identity_list_request(sock) < 0) {
		dprintf(STDERR_FILENO, "ERROR: Failed to send identity list request\n");
		goto exiting;
	}
	response = kc_ssha_get_full_response(sock);
	if (response == NULL) {
		dprintf(STDERR_FILENO, "ERROR: Could not get response for identity list request\n");
		goto exiting;
	}

	if (agent_failed(response->type)) {
		dprintf(STDERR_FILENO, "ERROR: SSH agent request failed\n");
		goto exiting;
	} else if (response->type != SSH2_AGENT_IDENTITIES_ANSWER) {
		dprintf(STDERR_FILENO, "ERROR: Invalid SSH agent response type\n");
		goto exiting;
	}

	idlist = kc_ssha_parse_identities(response);
	if (idlist == NULL) {
		dprintf(STDERR_FILENO, "ERROR: Could not parse identity list\n");
		goto exiting;
	}

	do {
		if (getenv("KC_DEBUG"))
			printf("%s(): ('%s' ?= '%s') '%s' ?= '%s'\n", __func__, db_params->ssha_type, idlist->type, db_params->ssha_comment, idlist->comment);

		if (	strncmp(db_params->ssha_type, idlist->type, strlen(idlist->type)) == 0  &&
			strncmp(db_params->ssha_comment, idlist->comment, strlen(idlist->comment)) == 0) {

			if (getenv("KC_DEBUG"))
				printf("%s(): found a match for identity: (%s) %s\n", __func__, db_params->ssha_type, db_params->ssha_comment);

			break;
		}

		free(idlist->pubkey); idlist->pubkey = NULL;
		free(idlist->type); idlist->type = NULL;
		free(idlist->comment); idlist->comment = NULL;

		idlist_prev = idlist;
		idlist = idlist->next;

		free(idlist_prev); idlist_prev = NULL;
	} while (idlist != NULL);

	if (idlist == NULL) {
		dprintf(STDERR_FILENO, "ERROR: Could not find a match for identity: (%s) %s\n", db_params->ssha_type, db_params->ssha_comment);
		goto exiting;
	}


	/* set up data to be signed */
	data_to_sign = malloc(IV_DIGEST_LEN + SALT_DIGEST_LEN + (db_params->ssha_password ? db_params->pass_len : 0)); malloc_check(data_to_sign);

	memcpy(data_to_sign, db_params->iv, IV_DIGEST_LEN);
	data_to_sign_len += IV_DIGEST_LEN;

	memcpy(data_to_sign + data_to_sign_len, db_params->salt, SALT_DIGEST_LEN);
	data_to_sign_len += SALT_DIGEST_LEN;

	/* if a password is also part of the data to sign, i.e. non-automatic mode */
	if (db_params->ssha_password) {
		if (getenv("KC_DEBUG"))
			printf("%s(): using a password in the data to sign\n", __func__);

		memcpy(data_to_sign + data_to_sign_len, db_params->pass, db_params->pass_len);
		data_to_sign_len += db_params->pass_len;

		/* save the password temporarily, so that we can append it
		 * later after the signature in the new constructed password */
		passtmp = malloc(db_params->pass_len); malloc_check(passtmp);
		passtmp_len = db_params->pass_len;
		memcpy(passtmp, db_params->pass, db_params->pass_len);
	}

	/* ask for a signature */
	if (kc_ssha_sign_request(sock, idlist, data_to_sign, data_to_sign_len, 0) < 0) {
		dprintf(STDERR_FILENO, "ERROR: Failed to send signature request\n");
		goto exiting;
	}

	response = kc_ssha_get_full_response(sock);
	if (response == NULL) {
		dprintf(STDERR_FILENO, "ERROR: Could not get response for signature request\n");
		goto exiting;
	}

	if (agent_failed(response->type)) {
		dprintf(STDERR_FILENO, "ERROR: SSH agent request failed\n");
		goto exiting;
	} else if (response->type != SSH2_AGENT_SIGN_RESPONSE) {
		dprintf(STDERR_FILENO, "ERROR: Invalid SSH agent response type\n");
		goto exiting;
	}

	signature = kc_ssha_parse_signature(response);
	if (signature == NULL) {
		dprintf(STDERR_FILENO, "ERROR: Could not parse signature response\n");
		goto exiting;
	}

	if (signature->length > PASSWORD_MAXLEN) {
		dprintf(STDERR_FILENO, "ERROR: Signature length(%zd) is larger than the allowed password length(%d)\n", signature->length, PASSWORD_MAXLEN);
		goto exiting;
	}

	/* realloc ..->pass */
	memset(db_params->pass, '\0', db_params->pass_len);
	db_params->pass_len = signature->length + passtmp_len > PASSWORD_MAXLEN ? PASSWORD_MAXLEN : signature->length + passtmp_len;
	db_params->pass = realloc(db_params->pass, db_params->pass_len); malloc_check(db_params->pass);

	/* copy the signature as the constructed password */
	memcpy(db_params->pass, signature->signature, signature->length);

	/* if there's a password, then append it to the signature */
	if (db_params->ssha_password  &&  passtmp) {
		if (getenv("KC_DEBUG"))
			printf("%s(): constructing new password by appending password to signature\n", __func__);

		memcpy(db_params->pass + signature->length, passtmp, signature->length + passtmp_len > db_params->pass_len ? db_params->pass_len - signature->length : passtmp_len);
	}

	ret = 1;

exiting:
	if (passtmp)
		memset(passtmp, '\0', passtmp_len);
	free(passtmp); passtmp = NULL;
	passtmp_len = 0;

	free(data_to_sign); data_to_sign = NULL;

	if (signature != NULL) {
		memset(signature->signature, '\0', signature->length);
		free(signature->signature); signature->signature = NULL;
		free(signature); signature = NULL;
	}

	if (response != NULL) {
		memset(response->data, '\0', response->length);
		free(response->data); response->data = NULL;
		free(response); response = NULL;
	}

	while (idlist != NULL) {
		free(idlist->pubkey); idlist->pubkey = NULL;
		free(idlist->type); idlist->type = NULL;
		free(idlist->comment); idlist->comment = NULL;

		idlist_prev = idlist;
		idlist = idlist->next;

		free(idlist_prev); idlist_prev = NULL;
	}

	if (sock > 0)
		close(sock);


	return(ret);
} /* kc_ssha_get_password() */
