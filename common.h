#ifndef _COMMON_H
#define _COMMON_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#ifdef _LINUX
#include <bsd/string.h>
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
#define	VERSION	"2.4-dev-SVN_VERSION"

#define	PASSWORD_MAXLEN	64
#define	IV_LEN		16
#define	SALT_LEN	16
#define	KEY_LEN		128

#define	TIME_MAXLEN	11

enum {
	KC_SETUP_CRYPT_IV = 1,
	KC_SETUP_CRYPT_SALT = 1 << 1,
	KC_SETUP_CRYPT_KEY = 1 << 2
};

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
