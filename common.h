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

#include <histedit.h>


#define	VERSION	"kc 1.0"


const char *e_prompt(EditLine *);
const char *e_prompt_null(EditLine *);
int malloc_check(void *);
char *get_random_str(int, char);
void quit(EditLine *, History *, BIO *, int);


char		debug;


#endif
