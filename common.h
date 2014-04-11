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

#include <openssl/bio.h>
#include <openssl/evp.h>

#ifndef _READLINE
#include <histedit.h>
#else
#include <readline/readline.h>
#include <readline/history.h>
#endif


#define	NAME	"kc"
#define	VERSION	"2.4.0-rc1"

#define	PASSWORD_MAXLEN	256
#define	IV_LEN		64
#define	IV_DIGEST_LEN	128
#define	SALT_LEN	64
#define	SALT_DIGEST_LEN	128
#define	KEY_LEN		128

#define	TIME_MAXLEN	11

#define	ITEMS_MAX	ULONG_MAX


enum {
	KC_SETUP_CRYPT_IV = 1,
	KC_SETUP_CRYPT_SALT = 1 << 1,
	KC_SETUP_CRYPT_KEY = 1 << 2
};

typedef struct db_parameters {
	char		*db_filename;
	int		db_file;
	char		*pass_filename;
	char 		*pass;
	char		*kdf;
	char		*cipher;
	char		*cipher_mode;
	unsigned char	iv[IV_DIGEST_LEN + 1];
	unsigned char	salt[SALT_DIGEST_LEN + 1];
	unsigned char	key[KEY_LEN];
	unsigned char	dirty;
	unsigned char	readonly;
} db_parameters;


#define	KC_DTD	"\
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
