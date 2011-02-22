#ifndef _COMMANDS_H
#define _COMMANDS_H


typedef struct command {
	char *name;
	char *usage;
	char *help;
	void (*fn)(EditLine *, ...);
	struct command *next;
} command;


void commands_init(command **);

void cmd_match(EditLine *, const char *, History *, BIO *);

void cmd_quit(EditLine *, ...);
void cmd_list(EditLine *, ...);
void cmd_cnew(EditLine *, ...);
void cmd_cdel(EditLine *, ...);
void cmd_cren(EditLine *, ...);
void cmd_clist(EditLine *, ...);
void cmd_c(EditLine *, ...);
void cmd_search(EditLine *, ...);
void cmd_searchre(EditLine *, ...);
void cmd_getnum(EditLine *, ...);
void cmd_new(EditLine *, ...);
void cmd_edit(EditLine *, ...);
void cmd_copy(EditLine *, ...);
void cmd_del(EditLine *, ...);
void cmd_write(EditLine *, ...);
void cmd_export(EditLine *, ...);
void cmd_import(EditLine *, ...);
void cmd_random(EditLine *, ...);
void cmd_clear(EditLine *, ...);
void cmd_version(EditLine *, ...);
void cmd_help(EditLine *, ...);

xmlNodePtr find_keychain(xmlChar *);
xmlNodePtr find_key(int);
xmlChar *parse_newlines(xmlChar *, char);
int digit_length(int);
xmlChar *convert_utf8(xmlChar *, char);


#endif
