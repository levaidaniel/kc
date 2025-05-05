#ifndef _COMMON_H
#define _COMMON_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/param.h>

#if defined(__linux__)  ||  defined(__CYGWIN__)
#include <bsd/string.h>
#include <limits.h>
#endif

#ifdef BSD
#include <string.h>
#include <sys/limits.h>
#endif

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#include <libxml/encoding.h>
#include <libxml/xmlstring.h>
#include <libxml/xmlversion.h>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#if OPENSSL_VERSION_MAJOR >= 3
#include <openssl/provider.h>
#if OPENSSL_VERSION_MINOR >= 2
/* For Argon2id */
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <openssl/thread.h>
#include <openssl/kdf.h>
#endif
#endif

#ifndef _READLINE
#include <histedit.h>
#else
#include <readline/readline.h>
#include <readline/history.h>
#endif


#define	NAME		"kc"
#define	VERSION		"2.5-dev-GIT_VERSION"

#define	PASSWORD_MAXLEN	1024
#define	IV_LEN		64
#define	IV_DIGEST_LEN	128
#define	SALT_LEN	64
#define	SALT_DIGEST_LEN	128
#define	KEY_MIN_LEN	16
#define	KEY_MAX_LEN	32

#define KC_BCRYPT_PBKDF_ROUNDS		36
#define KC_PKCS_PBKDF2_ITERATIONS	100000
#ifdef _HAVE_ARGON2
#define KC_ARGON2ID_ITERATIONS		2
#define KC_ARGON2ID_LANES		4
#define KC_ARGON2ID_MEMCOST		6291456
#endif

#define	TIME_MAXLEN	11

#define	ITEMS_MAX	ULONG_MAX

#define	DEFAULT_DB_DIR	".kc"
#define	DEFAULT_DB_FILENAME	"default.kcd"
#define	DEFAULT_KDF	"sha512"
#define	DEFAULT_CIPHER	"aes256"
#define	DEFAULT_MODE	"cbc"


typedef struct yk_array {
	char		dev;
	char		slot;
	unsigned int	serial;
	struct yk_array *next;
} yk_array;

typedef struct db_parameters {
	char		*db_filename;
	int		db_file;
	char		ssha_type[12];	/* 11 bytes for the longest supported 'ssh-ed25519' key type */
	char		ssha_comment[513];
	char		ssha_password;
	yk_array	*yk;
	unsigned char	yk_password;
	char		*pass_filename;
	char 		*pass;
	size_t 		pass_len;
	char		*kdf;
	unsigned long int	key_len;
	unsigned long int	kdf_reps;
	char		*cipher;
	char		*cipher_mode;
	void		*first;
	void		*second;
	void		*third;
	void		*fourth;
	void		*fifth;
	unsigned char	iv[IV_DIGEST_LEN + 1];
	unsigned char	salt[SALT_DIGEST_LEN + 1];
	unsigned char	*key;
	unsigned char	dirty;
	unsigned char	readonly;
} db_parameters;

typedef struct extra_parameters {
	char		*caller;

	/* for main() */
	char		*keychain_start;
	char		keychain_start_name;

	/* for cmd_export() */
	xmlChar		*cname;

	/* for cmd_import() */
	char		legacy;
} extra_parameters;


#define	KC_DTD	"\
<!ELEMENT kc (keychain)*> \
\
<!ELEMENT keychain (key)*> \
<!ATTLIST keychain \
	name CDATA #REQUIRED \
	description CDATA #REQUIRED \
	created CDATA #REQUIRED \
	modified CDATA #REQUIRED> \
\
<!ELEMENT key EMPTY> \
<!ATTLIST key \
	name CDATA #REQUIRED \
	value CDATA #REQUIRED \
	created CDATA #REQUIRED \
	modified CDATA #REQUIRED> \
"

#define	KC_DTD_LEGACY	"\
<!ELEMENT kc (keychain)*> \
\
<!ELEMENT keychain (key)*> \
<!ATTLIST keychain \
	name CDATA #REQUIRED \
	description CDATA #IMPLIED \
	created CDATA #IMPLIED \
	modified CDATA #IMPLIED> \
\
<!ELEMENT key EMPTY> \
<!ATTLIST key \
	name CDATA #REQUIRED \
	value CDATA #REQUIRED \
	created CDATA #IMPLIED \
	modified CDATA #IMPLIED> \
"


#ifndef _READLINE
const char	*el_prompt_null(void);
#endif
const char	*prompt_str(void);
int		malloc_check(void *);
void		version(void);
void		quit(int);


#endif
