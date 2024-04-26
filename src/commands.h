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

#ifndef _READLINE
void cmd_match(const char *);
#else
void cmd_match(char *);
#endif

void cmd_c(const char *, command *);
void cmd_cdel(const char *, command *);
void cmd_cedit(const char *, command *);
void cmd_clear(const char *, command *);
void cmd_clipboard(const char *, command *);
void cmd_clist(const char *, command *);
void cmd_cnew(const char *, command *);
void cmd_copy(const char *, command *);
void cmd_del(const char *, command *);
void cmd_edit(const char *, command *);
void cmd_export(const char *, command *);
void cmd_getnum(const unsigned long int, const unsigned long int);
void cmd_help(const char *, command *);
void cmd_import(const char *, command *);
void cmd_info(const char *, command *);
void cmd_list(const char *, command *);
void cmd_near(const char *, command *);
void cmd_new(const char *, command *);
void cmd_passwd(const char *, command *);
void cmd_quit(const char *, command *);
void cmd_search(const char *, command *);
void cmd_searchre(const char *, command *);
void cmd_status(const char *, command *);
void cmd_swap(const char *, command *);
void cmd_version(const char *, command *);
void cmd_write(const char *, command *);

xmlNodePtr	find_keychain(const xmlChar *, unsigned char);
xmlNodePtr	find_key(const unsigned long int);
void		_rl_push_buffer(void);
char		*get_random_str(const unsigned int, const unsigned char);
xmlChar		*parse_randoms(const xmlChar *);
xmlChar		*get_line(const xmlChar *, const unsigned long int);
xmlChar		*parse_newlines(const xmlChar *, const unsigned char);
unsigned long int count_elements(xmlNodePtr);
void		larg(char *, char ***, int *);
char		kc_password_read(struct db_parameters *, const unsigned char);
char		kc_crypt_iv_salt(struct db_parameters *);
char		kc_crypt_pass(struct db_parameters *, const unsigned char);
char		kc_crypt_key(struct db_parameters *);
char		kc_crypt_setup(BIO *, const unsigned int, struct db_parameters *);
BIO		*kc_setup_bio_chain(const char *, const unsigned char);
char		kc_db_writer(xmlDocPtr, BIO *, struct db_parameters *);
char		kc_validate_xml(xmlDocPtr, char);
int		kc_db_reader(char **, BIO *);
signed char	kc_arg_parser(int, char **, const char *, db_parameters *, extra_parameters *);


#endif
