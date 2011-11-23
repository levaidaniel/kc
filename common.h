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


#define	USAGE	"[-k database file] [-p password file] [-m cipher mode] [-b] [-v] [-h] [-d]"

#define	VERSION	"kc 2.1"


#ifndef _READLINE
const char *el_prompt_null(void);
#endif
const char *prompt_str(void);
int malloc_check(void *);
char *get_random_str(size_t, char);
void version(void);
void quit(int);


char		debug;


#endif
