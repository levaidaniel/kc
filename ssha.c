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


	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;

	sock_name = getenv("SSH_AUTH_SOCK");
	if (getenv("KC_DEBUG"))
		printf("SSH agent UNIX socket: '%s'\n", sock_name);
	if (sock_name == NULL  ||  strlen(sock_name) <= 0) {
		dprintf(STDERR_FILENO, "SSH agent socket name is NULL or zero length\n");
		return(-1);
	}

	if (strlcpy(addr.sun_path, sock_name, sizeof(addr.sun_path)) >= sizeof(addr.sun_path)) {
		dprintf(STDERR_FILENO, "Truncated strlcpy() when copying sock_name to sun_path\n");
		return(-1);
	}

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock <= 0) {
		dprintf(STDERR_FILENO, "Could not create UNIX socket for SSH agent communication\n");
		return(-1);
	}

	if (connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) != 0) {
		dprintf(STDERR_FILENO, "Could not connect to SSH agent UNIX socket: '%s'\n", sock_name);
		return(-1);
	}

	return(sock);
} /* kc_ssha_connect() */


char
kc_ssha_get_identity_list(int sock)
{
	char	*buf = NULL;
	char	request = SSH2_AGENTC_REQUEST_IDENTITIES;
	size_t	w = 0, buf_len = 1024;


	buf = calloc(1, buf_len); malloc_check(buf);

	/* my request's length */
	put_u32(buf, sizeof(request));
	w = write(sock, buf, 4);
	if (w != 4 ) {
		perror("socket write() error");
		free(buf); buf = NULL;
		return(-1);
	} else {
		/* reset the send buffer */
		memset(buf, 0, buf_len);
	}

	/* my request */
	memcpy(buf, &request, 1);
	w = write(sock, buf, 1);
	if (w != 1 ) {
		perror("socket write() error");
		free(buf); buf = NULL;
		return(-1);
	}


	free(buf); buf = NULL;
	return(1);
} /* kc_ssha_get_identity_list() */


char
kc_ssha_sign_request(int sock, struct kc_ssha_identity *id, char *data, unsigned int flags)
{
	char	*buf = NULL;
	char	len[4];
	char	request = SSH2_AGENTC_SIGN_REQUEST;
	size_t	w = 0, wt = 0, buf_len = 0, pos = 0, req_len = 0;


	if (data == NULL) {
		dprintf(STDERR_FILENO, "Data to sign is empty\n");
		return(-1);
	}

	/* 5 is the request's length field (without the flags field), plus
	 * request that is one byte
	 * 4 is the flags field */
	req_len = 5 + 4 + id->pubkey_len + 4 + strlen(data);
	buf_len = req_len + 4;	/* plus flags field */

	buf = calloc(1, buf_len); malloc_check(buf);

	/* my request's length */
	put_u32(len, req_len);
	memcpy(buf+pos, len, 4);
	memset(len, 0, 4);
	pos += 4;

	/* my request */
	memcpy(buf+pos, &request, 1);
	pos += 1;

	/* my key's length */
	put_u32(len, id->pubkey_len);
	memcpy(buf+pos, len, 4);
	memset(len, 0, 4);
	pos += 4;

	/* pubkey */
	memcpy(buf+pos, id->pubkey, id->pubkey_len);
	pos += id->pubkey_len;

	/* my data's length */
	put_u32(len, strlen(data));
	memcpy(buf+pos, len, 4);
	memset(len, 0, 4);
	pos += 4;

	/* data to sign */
	memcpy(buf+pos, data, strlen(data));
	pos += strlen(data);

	/* flags */
	/* This is not 'len' per se, I'm just using this variable. This is the
	 * flags parameter which is an uint32 */
	put_u32(len, 0);
	memcpy(buf+pos, len, 4);
	pos += 4;

	if (buf_len != pos) {
		dprintf(STDERR_FILENO, "Total copied bytes is different from the buffer size\n");
		free(buf); buf = NULL;
		return(-1);
	}

	pos = 0;
	do {
		w = write(sock, buf+pos, buf_len-pos);
		if (w <= 0) {
			perror("socket write() error");
			free(buf); buf = NULL;
			return(-1);
		}

		pos += w;
		wt += w;
	} while (pos < buf_len);

	if (getenv("KC_DEBUG"))
		dprintf(STDERR_FILENO, "wrote total = %zd\n", wt);


	free(buf); buf = NULL;
	return(1);
} /* kc_ssha_sign_request() */


struct kc_ssha_response*
kc_ssha_get_full_response(int sock)
{
	char	*buf = NULL, *nbuf = NULL;
	size_t	buf_len_base = MAX_AGENT_REPLY_LEN/256, buf_len_mult = 1;
	size_t	buf_len = buf_len_mult * buf_len_base, buf_len_prev = buf_len;
	size_t	buf_rem = buf_len, r = 0, rt = 0;
	size_t	len = 0, rem = len;

	struct kc_ssha_response	*response = NULL;


	buf_len = 10; buf_rem = buf_len;	/* XXX DEBUG */
	buf = malloc(buf_len); malloc_check(buf);

	/* first read the response's length */
	r = read(sock, buf, 4);
	if (r != 4 ) {
		perror("socket read() error");
		free(buf); buf = NULL;
		return(NULL);
	};

	len = get_u32(buf);
	if (getenv("KC_DEBUG"))
		dprintf(STDERR_FILENO, "response length = %zd\n", len);

	if (len > MAX_AGENT_REPLY_LEN) {
		dprintf(STDERR_FILENO, "Indicated response length(%zd) is more than MAX_AGENT_REPLY_LEN(%d)\n", len, MAX_AGENT_REPLY_LEN);
		free(buf); buf = NULL;
		return(NULL);
	} else {
		/* reset the send buffer */
		memset(buf, 0, buf_len);
	}

	rem = len;
	while (rem  &&  buf_rem  &&  buf_len < MAX_AGENT_REPLY_LEN) {
		if (rem > buf_rem) {
			buf_len_prev = buf_len;
			buf_len = ++buf_len_mult * buf_len_base;
			if (getenv("KC_DEBUG"))
				dprintf(STDERR_FILENO, "realloc response buf to %zd\n", buf_len);
			nbuf = realloc(buf, buf_len); malloc_check(nbuf);

			buf = nbuf;
			buf_rem += buf_len - buf_len_prev;

			continue;
		}

		r = read(sock, buf+(buf_len - buf_rem), rem);
		switch (r) {
			case -1:
				perror("socket read() error");
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
		dprintf(STDERR_FILENO, "read total = %zd\n", rt);

	if (rt != len) {
		dprintf(STDERR_FILENO, "Total read bytes is different from length indication in response\n");
		free(buf); buf = NULL;
		return(NULL);
	}


	/* create the response struct */
	response = malloc(sizeof(struct kc_ssha_response)); malloc_check(response);
	response->data = buf;
	response->length = rt;


	return(response);
} /* kc_ssha_get_full_response() */


struct kc_ssha_identity*
kc_ssha_parse_identities(struct kc_ssha_response *response)
{
	struct kc_ssha_identity		*idlist = NULL, *idlist_first = NULL;
	size_t	pos = 1;	/* begin with skipping response type */
	size_t	num_ids = 0, slen = 0;


	num_ids = get_u32(response->data+pos);
	if (getenv("KC_DEBUG"))
		dprintf(STDERR_FILENO, "SSH agent has %zd identities\n", num_ids);

	if (num_ids <= 0) {
		dprintf(STDERR_FILENO, "SSH agent has no identities\n");
		return(NULL);
	}
	pos += 4;

	do {
		if (idlist != NULL)	/* not first call */
			idlist = idlist->next;
		else
			idlist = calloc(1, sizeof(struct kc_ssha_identity)); malloc_check(idlist);

		if (idlist_first == NULL)	/* first call */
			idlist_first = idlist;

		/* pubkey length */
		idlist->pubkey_len = get_u32(response->data+pos);
		if (idlist->pubkey_len > KC_MAX_PUBKEY_LEN) {
			dprintf(STDERR_FILENO, "Public key length is larger than allowed\n");
			return(NULL);
		}
		pos += 4;

		/* public key */
		idlist->pubkey = calloc(1, idlist->pubkey_len); malloc_check(idlist->pubkey);
		memcpy(idlist->pubkey, response->data+pos, idlist->pubkey_len);
		pos += idlist->pubkey_len;

		/* key type
		 * get the key-type string's length and extract the key type,
		 * which are part of the pubkey blob
		 */
		slen = get_u32(idlist->pubkey);
		idlist->type = calloc(1, slen + 1); malloc_check(idlist->type);
		memcpy(idlist->type, idlist->pubkey+4, slen); idlist->type[slen] = '\0';
		/*				^^^^^ + 4 here is the length indication of the key type string */

		/* key comment */
		slen = get_u32(response->data+pos);
		if (slen > KC_MAX_PUBKEY_COMMENT_LEN) {
			dprintf(STDERR_FILENO, "Public key comment length is larger than allowed\n");
			return(NULL);
		}
		pos += 4;
		idlist->comment = calloc(1, slen); malloc_check(idlist->comment);
		memcpy(idlist->comment, response->data+pos, slen);
		pos += slen;

		idlist->next = calloc(1, sizeof(struct kc_ssha_identity)); malloc_check(idlist->next);

		if (getenv("KC_DEBUG"))
			printf("parsed SSH ID: (%s) %s, %zd long\n", idlist->type, idlist->comment, idlist->pubkey_len);
	} while (--num_ids);
	free(idlist->next); idlist->next = NULL;


	return(idlist_first);
} /* kc_ssha_parse_identities() */


struct kc_ssha_signature *
kc_ssha_parse_signature(struct kc_ssha_response *response)
{
	size_t	pos = 1;	/* begin with skipping response type */
	size_t	len = 0;
	char	*s = NULL;
	struct kc_ssha_signature	*signature = NULL;

	len = get_u32(response->data+pos);
	pos += 4;
	if (getenv("KC_DEBUG"))
		printf("signature length is %zd\n", len);

	if (len > KC_MAX_SIGNATURE_LEN) {
		dprintf(STDERR_FILENO, "Signature length is larger than allowed\n");
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
kc_ssha_get_password(char *type, char *comment, struct db_parameters *db_params)
{
	int		sock = -1;
	char		response_type = 0;
	char		*data_to_sign = NULL;
	struct kc_ssha_response		*response = NULL;
	struct kc_ssha_identity		*idlist = NULL;
	struct kc_ssha_signature 	*signature = NULL;


	sock = kc_ssha_connect();
	if (sock < 0) {
		dprintf(STDERR_FILENO, "Couldn't establish UNIX socket connection\n");
		return(0);
	}

	/* get identities */
	if (kc_ssha_get_identity_list(sock) < 0) {
		dprintf(STDERR_FILENO, "Failed to send identity list request\n");
		return(0);
	}
	response = kc_ssha_get_full_response(sock);
	if (response == NULL) {
		dprintf(STDERR_FILENO, "Could not get response for identity list request\n");
		return(0);
	}

	response_type = (char)*response->data;
	if (getenv("KC_DEBUG"))
		printf("response type is %d\n", response_type);
	if (agent_failed(response_type)) {
		dprintf(STDERR_FILENO, "SSH agent request failed\n");
		return(0);
	} else if (response_type != SSH2_AGENT_IDENTITIES_ANSWER) {
		dprintf(STDERR_FILENO, "Invalid SSH agent response type\n");
		return(0);
	}

	idlist = kc_ssha_parse_identities(response);
	if (idlist == NULL) {
		dprintf(STDERR_FILENO, "Could not parse identity list\n");
		return(0);
	}

	do {
		if (	strncmp(type, idlist->type, strlen(idlist->type)) == 0  &&
			strncmp(comment, idlist->comment, strlen(idlist->comment)) == 0) {
			break;
		}

		idlist = idlist->next;
	} while (idlist != NULL);

	if (idlist == NULL) {
		dprintf(STDERR_FILENO, "Could not find a match for identity: (%s) %s\n", type, comment);
		return(0);
	}


	/* ask for a signature */
	data_to_sign = malloc(IV_DIGEST_LEN + SALT_DIGEST_LEN + 1); malloc_check(data_to_sign);
	if (strlcpy(data_to_sign, (const char*)db_params->iv, IV_DIGEST_LEN + 1) >= IV_DIGEST_LEN + 1) {
		dprintf(STDERR_FILENO, "Error while setting up SSH agent signing.\n");
		free(data_to_sign); data_to_sign = NULL;
		return(0);
	}
	if (strlcat(data_to_sign, (const char*)db_params->salt, SALT_DIGEST_LEN + IV_DIGEST_LEN + 1) >= SALT_DIGEST_LEN + IV_DIGEST_LEN + 1) {
		dprintf(STDERR_FILENO, "Error while setting up SSH agent signing.\n");
		free(data_to_sign); data_to_sign = NULL;
		return(0);
	}

	if (getenv("KC_DEBUG"))
		printf("data to sign is '%s'\n", data_to_sign);

	if (kc_ssha_sign_request(sock, idlist, data_to_sign, 0) < 0) {
		dprintf(STDERR_FILENO, "Failed to send signature request\n");
		free(data_to_sign); data_to_sign = NULL;
		return(0);
	}
	free(data_to_sign); data_to_sign = NULL;

	response = kc_ssha_get_full_response(sock);
	if (response == NULL) {
		dprintf(STDERR_FILENO, "Could not get response for signature request\n");
		return(0);
	}

	response_type = (char)*response->data;
	if (getenv("KC_DEBUG"))
		printf("response type is %d\n", response_type);
	if (agent_failed(response_type)) {
		dprintf(STDERR_FILENO, "SSH agent request failed\n");
		return(0);
	} else if (response_type != SSH2_AGENT_SIGN_RESPONSE) {
		dprintf(STDERR_FILENO, "Invalid SSH agent response type\n");
		return(0);
	}

	signature = kc_ssha_parse_signature(response);
	if (signature == NULL) {
		dprintf(STDERR_FILENO, "Could not parse signature response\n");
		return(0);
	}

	if (signature->length > PASSWORD_MAXLEN) {
		dprintf(STDERR_FILENO, "Signature length(%zd) is larger than the allowed password length(%d)\n", signature->length * 2, PASSWORD_MAXLEN);
		return(0);
	}

	db_params->pass_len = signature->length;
	db_params->pass = malloc(db_params->pass_len); malloc_check(db_params->pass);
	memcpy(db_params->pass, signature->signature, db_params->pass_len);

	memset(signature->signature, 0, signature->length);
	free(signature->signature); signature->signature = NULL;
	free(signature); signature = NULL;


	return(1);
} /* kc_ssha_get_password() */
