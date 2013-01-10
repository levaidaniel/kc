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
#define	VERSION	"2.2.0"
#define	USAGE	"[-k database file] [-r] [-p password file] [-m cipher mode] [-b] [-v] [-h]"

#define	PASSWORD_MAXLEN	64

enum {
	KC_GENERATE_IV = 1,
	KC_GENERATE_SALT = 1 << 1,
	KC_GENERATE_KEY = 1 << 2
};

#ifndef _READLINE
const char	*el_prompt_null(void);
#endif
const char	*prompt_str(void);
int		malloc_check(void *);
void		version(void);
void		quit(int);


#endif
