#ifndef _COMMANDS_H
#define _COMMANDS_H


typedef struct command command;
struct command {
	const char *name;
	const char *usage;
	const char *help;
	void (*fn)(const char *, command *);
	struct command *next;
};


void commands_init(command **);

void cmd_match(const char *);

void cmd_c(const char *, command *);
void cmd_cdel(const char *, command *);
void cmd_clear(const char *, command *);
void cmd_clist(const char *, command *);
void cmd_cnew(const char *, command *);
void cmd_copy(const char *, command *);
void cmd_cren(const char *, command *);
void cmd_del(const char *, command *);
void cmd_edit(const char *, command *);
void cmd_export(const char *, command *);
void cmd_getnum(int, size_t);
void cmd_help(const char *, command *);
void cmd_import(const char *, command *);
void cmd_list(const char *, command *);
void cmd_new(const char *, command *);
void cmd_quit(const char *, command *);
void cmd_random(const char *, command *);
void cmd_search(const char *, command *);
void cmd_searchre(const char *, command *);
void cmd_version(const char *, command *);
void cmd_write(const char *, command *);

xmlNodePtr find_keychain(xmlChar *);
xmlNodePtr find_key(int);
xmlChar *parse_newlines(xmlChar *, char);
size_t digit_length(int);
void _rl_push_buffer(void);


#endif
