#ifndef _COMMANDS_H
#define _COMMANDS_H


typedef struct command command;
struct command {
	const char *name;
	const char *usage;
	const char *help;
	void (*fn)(char *, command *);
	struct command *next;
};


void commands_init(command **);

void cmd_match(char *);

void cmd_quit(char *, command *);
void cmd_list(char *, command *);
void cmd_cnew(char *, command *);
void cmd_cdel(char *, command *);
void cmd_cren(char *, command *);
void cmd_clist(char *, command *);
void cmd_c(char *, command *);
void cmd_search(char *, command *);
void cmd_searchre(char *, command *);
void cmd_getnum(int, int);
void cmd_new(char *, command *);
void cmd_edit(char *, command *);
void cmd_copy(char *, command *);
void cmd_del(char *, command *);
void cmd_write(char *, command *);
void cmd_export(char *, command *);
void cmd_import(char *, command *);
void cmd_random(char *, command *);
void cmd_clear(char *, command *);
void cmd_version(char *, command *);
void cmd_help(char *, command *);

xmlNodePtr find_keychain(xmlChar *);
xmlNodePtr find_key(int);
xmlChar *parse_newlines(xmlChar *, char);
int digit_length(int);
xmlChar *convert_utf8(xmlChar *, char);
void _rl_push_buffer(void);


#endif
