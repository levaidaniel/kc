#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "common.h"
#include "ssha.h"


extern db_parameters	db_params;


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
	if (sock_name == NULL  ||  strlen(sock_name) <= 0) {
		dprintf(STDERR_FILENO, "sock_name is NULL or of zero length\n");
		return(-1);
	}

	if (strlcpy(addr.sun_path, sock_name, sizeof(addr.sun_path)) >= sizeof(addr.sun_path)) {
		dprintf(STDERR_FILENO, "truncated strlcpy() when copying sock_name to sun_path\n");
		return(-1);
	}

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock <= 0) {
		dprintf(STDERR_FILENO, "couldn't create UNIX socket\n");
		return(-1);
	}

	if (connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) != 0) {
		dprintf(STDERR_FILENO, "couldn't connect to UNIX socket: '%s'\n", sock_name);
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


	buf = calloc(1, buf_len);
	if (buf == NULL) {
		/* TODO Error message */
		return(-1);
	}

	/* my request's length */
	put_u32(buf, sizeof(request));
	w = write(sock, buf, 4);
	dprintf(STDERR_FILENO, "written = %zd\n", w);
	if (w != 4 ) {
		perror("write() error");
		free(buf); buf = NULL;
		return(-1);
	} else {
		/* reset the send buffer */
		memset(buf, 0, buf_len);
	}

	/* my request */
	memcpy(buf, &request, 1);
	w = write(sock, buf, 1);
	dprintf(STDERR_FILENO, "written = %zd\n", w);
	if (w != 1 ) {
		perror("write() error");
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
		dprintf(STDERR_FILENO, "data to sign is NULL\n");
		return(-1);
	}

	/* 5 is the request's length field (without the flags field), plus
	 * request that is one byte
	 * 4 is the flags field */
	req_len = 5 + 4 + id->pubkey_len + 4 + strlen(data);
	buf_len = req_len + 4;	/* plus flags field */

	buf = calloc(1, buf_len);
	if (buf == NULL) {
		/* TODO Error message */
		return(-1);
	}

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

	dprintf(STDERR_FILENO, "buf_len was %zd and we've copied %zd\n", buf_len, pos);

	pos = 0;
	do {
		w = write(sock, buf+pos, buf_len-pos);
		dprintf(STDERR_FILENO, "wrote = %zd\n", w);
		if (w <= 0) {
			perror("write() error");
			free(buf); buf = NULL;
			return(-1);
		}

		pos += w;
		wt += w;
	} while (pos < buf_len);

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
	buf = malloc(buf_len);
	if (buf == NULL) {
		/* TODO Error message */
		return(NULL);
	}

	/* first read the response's length */
	r = read(sock, buf, 4);
	dprintf(STDERR_FILENO, "read = %zd\n", r);
	if (r <= 0 ) {
		perror("read() error");
		free(buf); buf = NULL;
		return(NULL);
	};
	len = get_u32(buf);
	dprintf(STDERR_FILENO, "len = %zd\n", len);
	if (len > MAX_AGENT_REPLY_LEN) {
		dprintf(STDERR_FILENO, "len(%zd) is more than MAX_AGENT_REPLY_LEN(%d)\n", len, MAX_AGENT_REPLY_LEN);
		free(buf); buf = NULL;
		return(NULL);
	} else {
		/* reset the send buffer */
		memset(buf, 0, buf_len);
	}

	rem = len;
	dprintf(STDERR_FILENO, "rem = %zd\n", rem);
	dprintf(STDERR_FILENO, "buf_len = %zd\n", buf_len);
	while (rem  &&  buf_rem  &&  buf_len < MAX_AGENT_REPLY_LEN) {
		if (rem > buf_rem) {
			buf_len_prev = buf_len;
			buf_len = ++buf_len_mult * buf_len_base;
			dprintf(STDERR_FILENO, "realloc buf to %zd\n", buf_len);
			nbuf = realloc(buf, buf_len);
			if (nbuf == NULL) {
				/* TODO Error message */
				free(buf); buf = NULL;
				return(NULL);
			}

			buf = nbuf;
			buf_rem += buf_len - buf_len_prev;
			dprintf(STDERR_FILENO, "buf_rem = %zd\n", buf_rem);

			continue;
		}

		r = read(sock, buf+(buf_len - buf_rem), rem);
		dprintf(STDERR_FILENO, "read = %zd\n", r);
		switch (r) {
			case -1:
				perror("read() error");
				free(buf); buf = NULL;
				return(NULL);
			break;
			case 0:
			default:
				dprintf(STDERR_FILENO, "read 0 (EOF)\n");
				rem -= r;	/* this is what remained in the socket */
				dprintf(STDERR_FILENO, "rem = %zd\n", rem);
				rt += r;	/* this is total bytes read */
				dprintf(STDERR_FILENO, "rt = %zd\n", rt);
				buf_rem -= r;	/* this is what my buffer can still hold */
				dprintf(STDERR_FILENO, "buf_rem = %zd\n", buf_rem);
			break;
		};
	}


	/* we didn't read as much as we should've */
	if (rt != len) {
		/* TODO Error message */
		free(buf); buf = NULL;
		return(NULL);
	}


	/* create the response struct */
	response = malloc(sizeof(struct kc_ssha_response));
	if (response == NULL) {
		/* TODO Error message */
		free(buf);
		return(NULL);
	}
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
	dprintf(STDERR_FILENO, "agent has %zd identities\n", num_ids);
	if (num_ids <= 0) {
		return(NULL);
	}
	pos += 4;

	do {
		if (idlist != NULL)	/* not first call */
			idlist = idlist->next;
		else
			idlist = calloc(1, sizeof(struct kc_ssha_identity));

		if (idlist_first == NULL)	/* first call */
			idlist_first = idlist;

		/* pubkey length */
		idlist->pubkey_len = get_u32(response->data+pos);
		if (idlist->pubkey_len > KC_MAX_PUBKEY_LEN) {
			dprintf(STDERR_FILENO, "pubkey length is larger than allowed\n");
			return(NULL);
		}
		pos += 4;

		/* public key */
		idlist->pubkey = calloc(1, idlist->pubkey_len);
		memcpy(idlist->pubkey, response->data+pos, idlist->pubkey_len);
		pos += idlist->pubkey_len;

		/* key type
		 * get the key-type string's length and extract the key type,
		 * which are part of the pubkey blob
		 */
		slen = get_u32(idlist->pubkey);
		idlist->type = calloc(1, slen + 1);
		memcpy(idlist->type, idlist->pubkey+4, slen); idlist->type[slen] = '\0';
		/*				^^^^^ + 4 here is the length indication of the key type string */

		/* key comment */
		slen = get_u32(response->data+pos);
		if (slen > KC_MAX_PUBKEY_COMMENT_LEN) {
			dprintf(STDERR_FILENO, "pubkey comment length is larger than allowed\n");
			return(NULL);
		}
		pos += 4;
		idlist->comment = calloc(1, slen);
		memcpy(idlist->comment, response->data+pos, slen);
		pos += slen;

		idlist->next = calloc(1, sizeof(struct kc_ssha_identity));
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
	dprintf(STDERR_FILENO, "signature length is %zd\n", len);

	if (len > KC_MAX_SIGNATURE_LEN) {
		dprintf(STDERR_FILENO, "signature length is larger than allowed\n");
		return(NULL);
	}
	s = calloc(1, len);
	memcpy(s, response->data+pos, len);

	signature = calloc(1, sizeof(struct kc_ssha_signature));
	signature->signature = s;
	signature->length = len;

	return(signature);
} /* kc_ssha_parse_signature() */


char *
kc_ssha_get_password(char *type, char *comment)
{
	int		sock = -1, i = 0;
	size_t		pos = 0;
	char		response_type = 0;
	char		*data_to_sign = NULL, *password = NULL;
	char		hex[3];
	struct kc_ssha_response		*response = NULL;
	struct kc_ssha_identity		*idlist = NULL;
	struct kc_ssha_signature 	*signature = NULL;


	data_to_sign = malloc(IV_DIGEST_LEN + SALT_DIGEST_LEN + 1); malloc_check(data_to_sign);
	dprintf(STDERR_FILENO, "Using iv for signing: '%s'\n", db_params.iv);
	dprintf(STDERR_FILENO, "Using salt for signing: '%s'\n", db_params.salt);
	if (strlcpy(data_to_sign, (const char*)db_params.iv, IV_DIGEST_LEN + 1) >= IV_DIGEST_LEN + 1) {
		puts("Error while setting up OpenSSH agent signing.");
		free(data_to_sign); data_to_sign = NULL;
		return(NULL);
	}
	dprintf(STDERR_FILENO, "data_to_sign: '%s'\n", data_to_sign);
	if (strlcat(data_to_sign, (const char*)db_params.salt, SALT_DIGEST_LEN + IV_DIGEST_LEN + 1) >= SALT_DIGEST_LEN + IV_DIGEST_LEN + 1) {
		puts("Error while setting up OpenSSH agent signing.");
		free(data_to_sign); data_to_sign = NULL;
		return(NULL);
	}
	dprintf(STDERR_FILENO, "data_to_sign: '%s'\n", data_to_sign);

	sock = kc_ssha_connect();
	if (sock < 0) {
		dprintf(STDERR_FILENO, "couldn't establish UNIX socket connection\n");
		return(NULL);
	}

	/* get identities */
	if (kc_ssha_get_identity_list(sock) < 0) {
		dprintf(STDERR_FILENO, "failed to send identity list request\n");
	}
	response = kc_ssha_get_full_response(sock);
	if (response == NULL) {
		/* TODO Error message */
		return(NULL);
	}

	response_type = (char)*response->data;
	dprintf(STDERR_FILENO, "response type is %d\n", response_type);
	if (agent_failed(response_type)) {
		printf("auth agent something failed\n");
	} else if (response_type != SSH2_AGENT_IDENTITIES_ANSWER) {
		printf("invalid response type\n");
	}

	idlist = kc_ssha_parse_identities(response);
	if (idlist == NULL) {
		/* TODO Error message */
		return(NULL);
	}

	do {
		if (	strncmp(type, idlist->type, strlen(idlist->type)) == 0  &&
			strncmp(comment, idlist->comment, strlen(idlist->comment)) == 0) {
			break;
		}

		idlist = idlist->next;
	} while (idlist != NULL);

	if (idlist == NULL) {
		printf("could not find a match for (%s) %s\n", type, comment);
		return(NULL);
	} else {
		printf("found a match for (%s) %s\n", type, comment);
	}


	/* ask for a signature */
	if (kc_ssha_sign_request(sock, idlist, data_to_sign, 0) < 0) {
		dprintf(STDERR_FILENO, "failed to send signature request\n");
		return(NULL);
	}
	response = kc_ssha_get_full_response(sock);
	if (response == NULL) {
		/* TODO Error message */
		return(NULL);
	}

	response_type = (char)*response->data;
	dprintf(STDERR_FILENO, "response type is %d\n", response_type);
	if (agent_failed(response_type)) {
		printf("auth agent something failed\n");
	} else if (response_type != SSH2_AGENT_SIGN_RESPONSE) {
		printf("invalid response type\n");
	}

	signature = kc_ssha_parse_signature(response);
	if (signature == NULL) {
		/* TODO Error message */
		return(NULL);
	}

	if (signature->length > PASSWORD_MAXLEN) {
		dprintf(STDERR_FILENO, "PASSWORD_MAXLEN is smaller than the signature length\n");
		return(NULL);
	}

	password = calloc(1, PASSWORD_MAXLEN); malloc_check(password);
	pos = 0;
	for (i = 0; i < signature->length; i++) {
		snprintf(hex, 3, "%02x", signature->signature[i]);
		memcpy(password+pos, &hex, 2);
		pos += 2;
	}
	dprintf(STDERR_FILENO, "password from agent signature is '%s'\n", password);

	return(password);
} /* kc_ssha_get_password() */
