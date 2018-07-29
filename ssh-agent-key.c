#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <bsd/string.h>

/* Messages for the authentication agent connection. */
#define SSH_AGENTC_REQUEST_RSA_IDENTITIES	1
#define SSH_AGENT_RSA_IDENTITIES_ANSWER		2
#define SSH_AGENTC_RSA_CHALLENGE		3
#define SSH_AGENT_RSA_RESPONSE			4
#define SSH_AGENT_FAILURE			5
#define SSH_AGENT_SUCCESS			6
#define SSH_AGENTC_ADD_RSA_IDENTITY		7
#define SSH_AGENTC_REMOVE_RSA_IDENTITY		8
#define SSH_AGENTC_REMOVE_ALL_RSA_IDENTITIES	9

/* private OpenSSH extensions for SSH2 */
#define SSH2_AGENTC_REQUEST_IDENTITIES		11
#define SSH2_AGENT_IDENTITIES_ANSWER		12
#define SSH2_AGENTC_SIGN_REQUEST		13
#define SSH2_AGENT_SIGN_RESPONSE		14
#define SSH2_AGENTC_ADD_IDENTITY		17
#define SSH2_AGENTC_REMOVE_IDENTITY		18
#define SSH2_AGENTC_REMOVE_ALL_IDENTITIES	19

/* smartcard */
#define SSH_AGENTC_ADD_SMARTCARD_KEY		20
#define SSH_AGENTC_REMOVE_SMARTCARD_KEY		21

/* lock/unlock the agent */
#define SSH_AGENTC_LOCK				22
#define SSH_AGENTC_UNLOCK			23

/* add key with constraints */
#define SSH_AGENTC_ADD_RSA_ID_CONSTRAINED	24
#define SSH2_AGENTC_ADD_ID_CONSTRAINED		25
#define SSH_AGENTC_ADD_SMARTCARD_KEY_CONSTRAINED 26

#define	SSH_AGENT_CONSTRAIN_LIFETIME		1
#define	SSH_AGENT_CONSTRAIN_CONFIRM		2

/* extended failure messages */
#define SSH2_AGENT_FAILURE			30

/* additional error code for ssh.com's ssh-agent2 */
#define SSH_COM_AGENT2_FAILURE			102

#define	SSH_AGENT_OLD_SIGNATURE			0x01
#define	SSH_AGENT_RSA_SHA2_256			0x02
#define	SSH_AGENT_RSA_SHA2_512			0x04

#define MAX_AGENT_IDENTITIES	2048		/* Max keys in agent reply */
#define MAX_AGENT_REPLY_LEN	(256 * 1024) 	/* Max bytes in agent reply */

/* macro to check for "agent failure" message */
#define agent_failed(x) \
    ((x == SSH_AGENT_FAILURE) || \
    (x == SSH_COM_AGENT2_FAILURE) || \
    (x == SSH2_AGENT_FAILURE))


struct kc_ssha_response {
	char	*data;
	size_t	length;
};

struct kc_ssha_identitylist {
	char	*type;
	void	*pubkey;
	char	*comment;
	void	*next;
};


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


void
print_buf(char *buf, ssize_t len)
{
	int		i = 0;


	printf("buf = ");
	for (i=0; i < len; i++) {
		printf("%02X", buf[i]);
	}
	puts("");
}


int
kc_ssha_connect()
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


struct kc_ssha_response*
kc_ssha_get_full_response(int sock, char request)
{
	char	*buf = NULL, *nbuf = NULL;
	size_t	buf_len_base = MAX_AGENT_REPLY_LEN/256, buf_len_mult = 1;
	size_t	buf_len = buf_len_mult * buf_len_base, buf_len_prev = buf_len;
	size_t	buf_rem = buf_len, r = 0, rt = 0;
	size_t	len = 0, rem = len;

	struct kc_ssha_response *response = NULL;


	buf_len = 10; buf_rem = buf_len;	/* XXX DEBUG */
	buf = malloc(buf_len);
	if (buf == NULL) {
		/* TODO Error message */
		return(NULL);
	}

	/* my request's length */
	put_u32(buf, sizeof(request));
	r = write(sock, buf, 4);
	dprintf(STDERR_FILENO, "written = %zd\n", r);
	if (r != 4 ) {
		perror("write() error");
		free(buf); buf = NULL;
		return(NULL);
	} else {
		/* reset the send buffer */
		memset(buf, 0, buf_len);
	}

	/* my request */
	memcpy(buf, &request, 1);
	r = write(sock, buf, 1);
	dprintf(STDERR_FILENO, "written = %zd\n", r);
	if (r != 1 ) {
		perror("write() error");
		free(buf); buf = NULL;
		return(NULL);
	} else {
		/* reset the send buffer */
		memset(buf, 0, buf_len);
	}


	/* read the full response */

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


struct kc_ssha_identitylist*
kc_ssha_parse_identities(struct kc_ssha_response *response)
{
	struct kc_ssha_identitylist	*idl = NULL, *idl_first = NULL;
	int	i = 0;
	size_t	pos = 1;	/* begin with skipping response type */
	size_t	num_ids = 0, slen = 0, id_len = 0;


	num_ids = get_u32(response->data+pos);
	dprintf(STDERR_FILENO, "agent has %zd identities\n", num_ids);
	if (num_ids <= 0) {
		return(NULL);
	}
	pos += 4;

	idl = calloc(1, sizeof(struct kc_ssha_identitylist));
	idl_first = idl;
	for (i=0; i < num_ids; i++) {
		/* id length */
		id_len = get_u32(response->data+pos);
		pos += 4;

		/* public key */
		idl->pubkey = (void *)calloc(1, id_len);
		memcpy(idl->pubkey, response->data+pos, id_len);
		pos += id_len;

		/* key type
		 * get the key-type string's length and extract the key type,
		 * which are part of the pubkey blob
		 */
		slen = get_u32(idl->pubkey);
		idl->type = (char *)calloc(1, slen + 1);
		memcpy(idl->type, idl->pubkey+4, slen); idl->type[slen] = '\0';
		/*			^^^^^^^ + 4 here is the length indication of the key type string */

		/* key comment */
		slen = get_u32(response->data+pos);
		pos += 4;
		idl->comment = (char *)calloc(1, slen);
		memcpy(idl->comment, response->data+pos, slen);
		pos += slen;

		idl->next = calloc(1, sizeof(struct kc_ssha_identitylist));
		idl = idl->next;
	}
	free(idl->next); idl->next = NULL;

	return(idl_first);
} /* kc_ssha_parse_identities() */


int
main(int argc, char *argv[])
{
	struct		ssh_identitylist *idlist;
	int		sock = -1, i = 0;

	char		agent_msg = 0;
	unsigned char	*msg = NULL;
	size_t		len = 0, num = 0, slen = 0;
	char		*buf = NULL, *tmp;
	ssize_t		r = -1, s = -1;
	char		response_type = 0;
	struct kc_ssha_response		*response = NULL;
	struct kc_ssha_identitylist	*idl = NULL;


	sock = kc_ssha_connect();
	if (sock < 0) {
		dprintf(STDERR_FILENO, "couldn't establish UNIX socket connection\n");
		return(1);
	}

	/* get identities */
	response = kc_ssha_get_full_response(sock, SSH2_AGENTC_REQUEST_IDENTITIES);
	if (response == NULL) {
		/* TODO Error message */
		return(1);
	}

	response_type = (char)*response->data;
	dprintf(STDERR_FILENO, "response type is %d\n", response_type);
	if (agent_failed(response_type)) {
		printf("auth agent something failed\n");
	} else if (response_type != SSH2_AGENT_IDENTITIES_ANSWER) {
		printf("invalid response type\n");
	}

	idl = kc_ssha_parse_identities(response);
	if (idl == NULL) {
		/* TODO Error message */
		return(1);
	}

	do {
		dprintf(STDERR_FILENO, "key type is '%s'\n", idl->type);
		dprintf(STDERR_FILENO, "key comment is '%s'\n", idl->comment);
		idl = idl->next;
	} while (idl->next != NULL);


	return(0);
} /* main() */
