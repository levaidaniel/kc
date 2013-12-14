/*
 * Copyright (c) 2011-2013 LEVAI Daniel
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *	* Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 *	* Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _LINUX
#include <fcntl.h>
#else
#include <sys/file.h>
#endif

#include "common.h"
#include "commands.h"


void		print_bio_chain(BIO *);
#ifndef _READLINE
unsigned char	el_tab_complete(EditLine *);
#else
char		*cmd_generator(const char *, int);
#endif


BIO		*bio_chain = NULL;

char		*cipher_mode = "cbc";

unsigned char	iv[IV_LEN + 1], salt[SALT_LEN + 1], key[KEY_LEN];

command		*commands_first = NULL;

xmlDocPtr	db = NULL;
xmlNodePtr	keychain = NULL;

int		db_file = 0;
char		*db_filename = NULL;

unsigned char	dirty = 0, batchmode = 0, readonly = 0;

#ifndef _READLINE
EditLine	*e = NULL;
History		*eh = NULL;
HistEvent	eh_ev;
#endif

char		prompt_context[30];


int
main(int argc, char *argv[])
{
#ifndef _READLINE
	int		e_count = 0;
#endif
	const char	*e_line = NULL;
	char		*line = NULL;

	char		*created = NULL;

	char		*buf = NULL;
	ssize_t		ret = -1;
	unsigned int	pos = 0;
	char		*pass = NULL;
	char		*pass_filename = NULL;
	int		pass_file = -1;
	size_t		pass_size = 128;
	int		kc_setup_crypt_flags = 0;

	struct stat	st;
	const char	*default_db_dir = ".kc";
	const char	*default_db_filename = "default.kcd";
	char		*env_home = NULL;
	unsigned char	newdb = 0;

	xmlNodePtr	db_root = NULL;

	int		c = 0;
	size_t		len = 0;


	while ((c = getopt(argc, argv, "k:rp:m:bvh")) != -1)
		switch (c) {
			case 'k':
				db_filename = optarg;
			break;
			case 'r':
				readonly = 1;
			break;
			case 'p':
				pass_filename = optarg;
			break;
			case 'm':
				cipher_mode = optarg;
			break;
			case 'b':
				batchmode = 1;
			break;
			case 'v':
				version();
				exit(EXIT_SUCCESS);
			break;
			case 'h':
				/* FALLTHROUGH */
			default:
				printf(	"%s [-k <database file>] [-r] [-p <password file>] [-m <cipher mode>] [-b] [-v] [-h]\n\n"
					"-k <file>: Use file as database. The default is ~/.kc/default.kcd .\n"
					"-r: Open the database in read-only mode.\n"
					"-p <file>: Read password from file.\n"
					"-m <cipher>: Cipher mode: cbc (default), cfb128, ofb.\n"
					"-b: Batch mode: disable some features to enable commands from standard input.\n"
					"-v: Display version.\n"
					"-h: This help.\n", argv[0]);
				exit(EXIT_SUCCESS);
			break;
		}


	/* Check if cipher mode is valid */
	if (	strcmp(cipher_mode, "cfb128") != 0  &&
		strcmp(cipher_mode, "ofb") != 0  &&
		strcmp(cipher_mode, "cbc") != 0) {

		printf("Unknown cipher mode: %s!\n", cipher_mode);

		quit(EXIT_FAILURE);
	}


	if (!db_filename) {
		/* using default database */

		env_home = getenv("HOME");

		len = strlen(env_home) + 1 + strlen(default_db_dir) + 1;
		db_filename = malloc(len); malloc_check(db_filename);

		/* default db directory (create it, if it doesn't exist) */
		snprintf(db_filename, len, "%s/%s", env_home, default_db_dir);

		if(stat(db_filename, &st) == 0) {
			if(!S_ISDIR(st.st_mode)) {
				printf("'%s' is not a directory!\n", db_filename);
				quit(EXIT_FAILURE);
			}
		} else {
			if (getenv("KC_DEBUG"))
				printf("creating '%s' directory\n", db_filename);

			if(mkdir(db_filename, 0777) != 0) {
				printf("Could not create '%s': %s\n", db_filename, strerror(errno));
				quit(EXIT_FAILURE);
			}
		}

		/* default db filename */
		len += 1 + strlen(default_db_filename);
		db_filename = realloc(db_filename, len); malloc_check(db_filename);

		snprintf(db_filename, len, "%s/%s/%s", env_home, default_db_dir, default_db_filename);
	}

	if (pass_filename) {	/* we were given a password file name */
		if (getenv("KC_DEBUG"))
			puts("opening password file");

		if(stat(pass_filename, &st) == 0) {
			if(!S_ISLNK(st.st_mode)  &&  !S_ISREG(st.st_mode)) {
				printf("'%s' is not a regular file or a link!\n", pass_filename);
				quit(EXIT_FAILURE);
			}
		} else {
			perror("stat(password file)");
			quit(EXIT_FAILURE);
		}

		/* read in the password from the specified file */
		pass_file = open(pass_filename, O_RDONLY);
		if (pass_file < 0) {
			perror("open(password file)");
			quit(EXIT_FAILURE);
		}

		pass = calloc(1, pass_size); malloc_check(pass);
		pos = 0;
		while (ret) {
			/* if we've reached the size of 'pass', grow it */
			if (pos >= pass_size) {
				pass_size += 128;
				pass = realloc(pass, pass_size); malloc_check(pass);
			}

			ret = read(pass_file, pass + pos, pass_size - pos);
			if (ret < 0) {
				perror("read(password file)");
				break;
			} else
				pos += ret;
		}
		pass[pos] = '\0';
		if (strrchr(pass, '\n'))
			pass[pos - 1] = '\0';		/* strip the newline character */

		if (close(pass_file) < 0)
			perror("close(password file)");
	} else {
		printf("Using '%s' database.\n", db_filename);

		if(stat(db_filename, &st) == 0)		/* if db_filename exists */
			/* ask for the password */
			kc_password_read(&pass, 0);
		else {
			/* ask for the new password */
			do {
				ret = kc_password_read(&pass, 1);
			} while (ret == -1);

			if (ret == 0)
				quit(EXIT_FAILURE);
		}
	}

	if(stat(db_filename, &st) == 0) {	/* if db_filename exists */
		newdb = 0;

		if(!S_ISLNK(st.st_mode)  &&  !S_ISREG(st.st_mode)) {
			printf("'%s' is not a regular file or a link!\n", db_filename);
			quit(EXIT_FAILURE);
		}
		printf("Opening '%s'\n", db_filename);

		db_file = open(db_filename, O_RDWR);
		if (db_file < 0) {
			perror("open(database file)");
			quit(EXIT_FAILURE);
		}

		/* read the IV */
		buf = malloc(IV_LEN + 1); malloc_check(buf);

		ret = read(db_file, buf, IV_LEN);
		if (ret < 0)
			perror("read(database file)");
		else
			strlcpy((char *)iv, (const char *)buf, IV_LEN + 1);

		free(buf); buf = NULL;

		/* read the salt */
		buf = malloc(SALT_LEN + 1); malloc_check(buf);

		ret = read(db_file, buf, SALT_LEN);
		if (ret < 0)
			perror("read(database file)");
		else
			strlcpy((char *)salt, (const char *)buf, SALT_LEN + 1);

		free(buf); buf = NULL;


		kc_setup_crypt_flags = KC_SETUP_CRYPT_KEY;
	} else {
		newdb = 1;

		printf("Creating '%s'\n", db_filename);

		db_file = open(db_filename, O_RDWR | O_CREAT, 0600);
		if (db_file < 0) {
			perror("open(database file)");
			quit(EXIT_FAILURE);
		}

		kc_setup_crypt_flags = KC_SETUP_CRYPT_IV | KC_SETUP_CRYPT_SALT | KC_SETUP_CRYPT_KEY;
	}


	bio_chain = kc_setup_bio_chain(db_filename);
	if (!bio_chain) {
		puts("Could not setup bio_chain!");
		quit(EXIT_FAILURE);
	}
	if (getenv("KC_DEBUG"))
		print_bio_chain(bio_chain);


	/* Optionally generate iv/salt.
	 * Setup cipher mode and turn on decrypting */
	if (!kc_setup_crypt(bio_chain, 0, cipher_mode, pass, iv, salt, key, kc_setup_crypt_flags)) {
		puts("Could not setup decrypting!");
		quit(EXIT_FAILURE);
	}


	if (pass)
		memset(pass, '\0', PASSWORD_MAXLEN);
	free(pass); pass = NULL;


	if (!readonly)
		if (flock(db_file, LOCK_NB | LOCK_EX) < 0) {
			if (getenv("KC_DEBUG"))
				puts("flock(database file)");

			puts("Could not lock the database file!\nMaybe another instance is using that database?");
			quit(EXIT_FAILURE);
		}


	if (newdb) {		/* new database? */
		if (getenv("KC_DEBUG"))
			puts("empty database file");

		/* create a new document */
		db = xmlNewDoc(BAD_CAST "1.0");
		if (!db) {
			puts("Could not create XML document!");
			quit(EXIT_FAILURE);
		}

		db_root = xmlNewNode(NULL, BAD_CAST "kc");	/* THE root node */
		if (!db_root) {
			puts("Could not create root node!");
			quit(EXIT_FAILURE);
		}
		xmlDocSetRootElement(db, db_root);

		xmlAddChild(db_root, xmlNewText(BAD_CAST "\n\t"));

		keychain = xmlNewNode(NULL, BAD_CAST "keychain");	/* the first keychain ... */
		if (!keychain) {
			puts("Could not create default keychain!");
			quit(EXIT_FAILURE);
		}
		xmlAddChild(db_root, keychain);
		xmlNewProp(keychain, BAD_CAST "name", BAD_CAST "default");	/* ... its name is "default" */

		created = malloc(TIME_MAXLEN); malloc_check(created);
		snprintf(created, TIME_MAXLEN, "%d", (int)time(NULL));

		xmlNewProp(keychain, BAD_CAST "created", BAD_CAST created);


		xmlAddChild(keychain, xmlNewText(BAD_CAST "\n\t"));

		xmlAddChild(db_root, xmlNewText(BAD_CAST "\n"));

		/* Mark the database dirty, as a new initial database was just created */
		dirty = 1;
	} else {
		pos = kc_db_reader(&buf, bio_chain);
		if (getenv("KC_DEBUG"))
			printf("read %d bytes\n", pos);

		if (BIO_get_cipher_status(bio_chain) == 0) {
			puts("Failed to decrypt database file!");
			quit(EXIT_FAILURE);
		}

		if (getenv("KC_DEBUG"))
			db = xmlReadMemory(buf, (int)pos, NULL, "UTF-8", XML_PARSE_NONET | XML_PARSE_PEDANTIC);
		else
			db = xmlReadMemory(buf, (int)pos, NULL, "UTF-8", XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
		if (!db) {
			puts("Could not parse XML document!");

			if (strcmp(cipher_mode, "cbc") != 0)
				puts("If you have specified cfb128 or ofb for chipher mode, this could also mean that either you have entered a wrong password or specified a cipher mode other than that the database was encrypted with!");

			quit(EXIT_FAILURE);
		}

		/* Validate the XML structure against our kc.dtd */
		if (!kc_validate_xml(db)) {
			printf("Not a valid kc XML structure ('%s')!\n", db_filename);
			quit(EXIT_FAILURE);
		}

		db_root = xmlDocGetRootElement(db);
		if (!db_root) {
			puts("Could not find root node!");
			quit(EXIT_FAILURE);
		}
		if (!db_root->children) {
			puts("Could not find first keychain!");
			quit(EXIT_FAILURE);
		}

		keychain = db_root->children->next;
		if (!keychain) {
			puts("Could not find first keychain!");
			quit(EXIT_FAILURE);
		}


		/* turn on encrypting */
		if (!kc_setup_crypt(bio_chain, 1, cipher_mode, NULL, iv, NULL, key, 0)) {
			puts("Could not setup encrypting!");
			quit(EXIT_FAILURE);
		}
	}
	free(buf); buf = NULL;


	/* init and start the CLI */

	if (!batchmode)
		signal(SIGINT, SIG_IGN);

#ifndef _READLINE
	/* init editline */
	e = el_init("kc", stdin, stdout, stderr);
	if (!e) {
		perror("el_init()");
		quit(EXIT_FAILURE);
	}

	/* init editline history */
	eh = history_init();
	if (!eh) {
		perror("history_init()");
		quit(EXIT_FAILURE);
	}
	if (history(eh, &eh_ev, H_SETSIZE, 100) < 0)
		fprintf(stderr, "history(H_SETSIZE): %s\n", eh_ev.str);

	if (history(eh, &eh_ev, H_SETUNIQUE, 1) < 0)
		fprintf(stderr, "history(H_SETUNIQUE): %s\n", eh_ev.str);

	/* setup editline/history parameters */
	if (el_set(e, EL_PROMPT, prompt_str) != 0)
		perror("el_set(EL_PROMPT)");

	if (el_set(e, EL_SIGNAL, 1) != 0)
		perror("el_set(EL_SIGNAL)");

	if (el_set(e, EL_HIST, history, eh) != 0)
		perror("el_set(EL_HIST)");

	if (el_set(e, EL_ADDFN, "TAB", "TAB completion", el_tab_complete) != 0)
		perror("el_set(EL_ADDFN)");

	if (el_set(e, EL_BIND, "\t", "TAB", NULL) != 0)
		perror("el_set(EL_BIND)");

	if (el_set(e, EL_BIND, "^R", "ed-redisplay", NULL) != 0)
		perror("el_set(EL_BIND)");

	if (el_set(e, EL_BIND, "^E", "ed-move-to-end", NULL) != 0)
		perror("el_set(EL_BIND)");

	if (el_set(e, EL_BIND, "^A", "ed-move-to-beg", NULL) != 0)
		perror("el_set(EL_BIND)");

	if (el_set(e, EL_BIND, "^L", "ed-clear-screen", NULL) != 0)
		perror("el_set(EL_BIND)");

	if (el_source(e, NULL) != 0) {
		if (errno != 0) {
			if (errno != ENOENT)
				perror("Error executing .editrc");
		} else
			printf("Error executing .editrc\n");
	}
	if (el_set(e, EL_EDITMODE, 1) != 0) {
		perror("el_set(EL_EDITMODE)");
		quit(EXIT_FAILURE);
	}
#else
	/* init readline */
	if (rl_initialize() != 0) {
		perror("rl_initialize()");
		quit(EXIT_FAILURE);
	}

	rl_catch_signals = 1;

	rl_readline_name = "kc";

	rl_completion_entry_function = cmd_generator;
#endif

	strlcpy(prompt_context, "", sizeof(prompt_context));


	/* create the command list */
	commands_init(&commands_first);

	if (readonly)
		puts("Database is read-only!");

	/* command loop */
	do {
#ifndef _READLINE
		e_line = el_gets(e, &e_count);
#else
		e_line = readline(prompt_str());
#endif
		if (e_line) {
			if (strlen(e_line) > 0) {
				line = strdup(e_line);
#ifndef _READLINE
				line[(long)(strlen(line) - 1)] = '\0';		/* remove the newline character from the end */
				history(eh, &eh_ev, H_ENTER, line);
#else
				add_history(line);
#endif
				cmd_match(line);

				free(line); line = NULL;
			}
		} else
			cmd_quit(NULL, NULL);
	} while (1);

	/* NOTREACHED */
	return(0);
} /* main() */


void
cmd_match(const char *e_line)
{
	int	idx = -1, space = -1;
	char	*line = NULL, *cmd = NULL;
	command	*commands = commands_first;


	if (e_line)
		line = strdup(e_line);
	else
		return;

	/*
	 * special case, if only a number was entered,
	 * we display the appropriate entry, and if there
	 * is another number after it, use it as space for jamming
	 */
	sscanf(line, "%d %d", &idx, &space);
	if (idx >= 0) {
		if (space >= 0)
			cmd_getnum(idx, (size_t)space);
		else
			cmd_getnum(idx, 0);
	} else {
		cmd = strtok(line, " ");
		if (!cmd) {	/* probably an empty line */
			free(line); line = NULL;
			return;
		}

		while(commands) {
			/* Find an exact match */
			if (strcmp(commands->name, cmd) == 0)
				break;

			/* '[c]search' and '[c]/' commands can have flags as suffixes. */
			if (strncmp("search", cmd, 6) == 0)
				if (strncmp(commands->name, cmd, 6) == 0)
					break;
			if (strncmp("csearch", cmd, 7) == 0)
				if (strncmp(commands->name, cmd, 7) == 0)
					break;

			if (strncmp("/", cmd, 1) == 0)
				if (strncmp(commands->name, cmd, 1) == 0)
					break;
			if (strncmp("c/", cmd, 2) == 0)
				if (strncmp(commands->name, cmd, 2) == 0)
					break;

			commands = commands->next;	/* iterate through the linked list */
		}

		if (commands)
			commands->fn(e_line, commands);	/* we call the command's respective function here */
		else
			printf("Unknown command: '%s'\n", cmd);
	}

	free(line); line = NULL;
} /* cmd_match() */


#ifndef _READLINE
const char *
el_prompt_null(void)
{
	return("");
}
#endif

const char *
prompt_str(void)
{
	size_t		prompt_len = 0;
	static char	*prompt = NULL;
	xmlChar		*cname = NULL;


	cname = xmlGetProp(keychain, BAD_CAST "name");

	prompt_len = 1 + (size_t)xmlStrlen(cname) + 2 + sizeof(prompt_context) + 2 + 1;
	prompt = realloc(prompt, prompt_len); malloc_check(prompt);

	snprintf(prompt, prompt_len, "<%s%% %s%c ", cname, prompt_context, (readonly ? '|':'>'));
	xmlFree(cname); cname = NULL;

	return(prompt);
} /* prompt_str() */


void
print_bio_chain(BIO *bio_chain)
{
	BIO	*bio_tmp = NULL;


	printf("bio_chain: ");
	bio_tmp = bio_chain;
	while (bio_tmp) {
		switch (BIO_method_type(bio_tmp)) {
			case (2|0x0400):
				printf("BIO_TYPE_FILE");
			break;
			case (8|0x0200):
				printf("BIO_TYPE_MD");
			break;
			case (10|0x0200):
				printf("BIO_TYPE_CIPHER");
			break;
			case (11|0x0200):
				printf("BIO_TYPE_BASE64");
			break;
			default:
				printf("%04X", BIO_method_type(bio_tmp));
			break;
		}
		bio_tmp = BIO_next(bio_tmp);
		if (!bio_tmp)
			break;
		printf("->");
	}
	puts("");
} /* print_bio_chain */


#ifndef _READLINE
unsigned char
el_tab_complete(EditLine *e)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*cname = NULL;

	char		*line_buf = NULL, *line_buf_copy = NULL,
			**match = NULL,
			*word_next = NULL, *word = NULL,
			*complete_max = NULL;
	const LineInfo	*el_lineinfo = NULL;
	command		*commands = commands_first;
	int		hits = 0, i = 0, j = 0, ref = 0, match_max = 0;
	size_t		word_len = 0;
	long		line_buf_len = 0;


	el_lineinfo = el_line(e);

	/*
	 * copy the buffer (ie. line) to our variable 'line_buf',
	 * because el_lineinfo->buffer is not NUL terminated
	 */
	line_buf_len = el_lineinfo->lastchar - el_lineinfo->buffer;
	line_buf = malloc((size_t)line_buf_len + 1); malloc_check(line_buf);
	memcpy(line_buf, el_lineinfo->buffer, (size_t)line_buf_len);
	line_buf[line_buf_len] = '\0';


	/*
	 * We search after match(es) for only the last word in the line buffer,
	 * and for this we are prepared for space separated words.
	 * If the last character is a space, then we treat it as if the previous
	 * word would have been finished, and there were no new one to complete.
	 * We complete 'word' to a command name or a keychain name, depending on
	 * its position in the line buffer.
	 *
	 * It's a side-effect, that if (word == NULL) and (word_len == 0) when
	 * searching for matches, then it will match for eveything with strncmp().
	 * So we will get a full list of command or keychain names which we wanted
	 * if the user is just pressing the completion key (eg. TAB) without something
	 * to be completed (eg. empty line, or right after a word and a space).
	 */

	if (line_buf_len > 0)
		if (line_buf[line_buf_len - 1] != ' ') {
			/*
			 * 'word' is the last word from 'line_buf'.
			 * 'line_buf' is the whole line entered so far to the edit buffer.
			 * we want to complete only the last word from the edit buffer.
			 */
			line_buf_copy = strdup(line_buf);	/* strtok() modifies its given string */
			word_next = strtok(line_buf_copy, " ");
			while (word_next) {
				word = word_next;
				word_next = strtok(NULL, " ");
			}

			if (word)
				word_len = strlen(word);
		}

	/*
	 * Altough it is okay if (word == NULL) as long as (word_len == 0),
	 * make this a bit nicer and place an empty string in word if it would be NULL.
	 */
	if (!word) {
		word_len = 0;
		word = "";
	}


	/*
	 * Basically, the first word will only be completed to a command.
	 * The second (or any additional) word will be completed to a command only
	 * if the first word was 'help' (because help [command] shows the help for command),
	 * or (if the first word is something other than 'help') it will be completed to a
	 * keychain name (to serve as a parameter for the previously completed command).
	 */

	/* only search for a command name if this is not an already completed command
	 * (ie. there is no space in the line), OR if the already completed command is 'help'
	 */
	if (!strchr(line_buf, ' ')  ||  (strncmp(line_buf, "help ", 5) == 0))
		/* search for a command name */
		do {
			if (strncmp(word, commands->name, word_len) == 0) {
				hits++;

				match = realloc(match, hits * sizeof(char *)); malloc_check(match);
				match[hits - 1] = strdup(commands->name);
			}
		} while((commands = commands->next));	/* iterate through the linked list */

	/* only search for a keychain name if this is an already completed command
	 * (ie. there is space(s) in the line), AND it's not the 'help' command that
	 * has been completed previously
	 */
	if (strchr(line_buf, ' ')  &&  (strncmp(line_buf, "help ", 5) != 0)) {
		/* search for a keychain name */
		db_node = keychain->parent->children;
		while (db_node) {
			if (db_node->type == XML_ELEMENT_NODE) {	/* we only care about ELEMENT nodes */
				cname = xmlGetProp(db_node, BAD_CAST "name");

				if (strncmp(word, (char *)cname, word_len) == 0) {
					hits++;

					match = realloc(match, hits * sizeof(char *)); malloc_check(match);
					match[hits - 1] = strdup((char *)cname);
				}

				xmlFree(cname); cname = NULL;
			}

			db_node = db_node->next;
		}
	}


	switch (hits) {
		case 0:	/* no match */
			printf("\a");
		break;
		case 1:
			/* print the word's remaining characters
			 * (remaining: the ones without the part (at the beginning)
			 * that we've entered already)
			 */
			el_push(e, match[0] + (int)word_len);

			/* if this is not an already completed word
			 * (ie. there is no space at the end), then
			 * put a space after the completed word
			 */
			if (line_buf[line_buf_len - 1] != ' ')
				el_push(e, " ");
		break;
		default:	/* more than one match */
			match_max = strlen(match[0]);	/* just a starting value */
			for (ref = 0; ref < hits; ref++)	/* for every matched command, ... */
				for (i = ref + 1; i < hits; i++) {	/* ... we compare it with the next and the remaining commands */
					j = 0;
					while (	match[ref][j] != '\0'  &&  match[i][j] != '\0'  &&
						match[ref][j] == match[i][j])
						j++;

					/* We are searching for the character that is the
					 * last common one in all of the matched commands.
					 *
					 * We are looking for the smallest number, the smallest
					 * difference between the user supplied command fragment
					 * and the shortest command name fragment that is common
					 * in all of the matched commands.
					 */
					if (j < match_max)
						match_max = j;
				}

			j = 0;
			/* complete_max will be the string difference between the user supplied command fragment
			 * and the shortest command name fragment that is common in all of the matched commands.
			 * eg.: user: "imp", matched commands: "import" and "importxml" => complete_max = "ort".
			 */
			complete_max = malloc(word_len + match_max + 1); malloc_check(complete_max);
			for (i = word_len; i < match_max; i++)
				complete_max[j++] = match[0][i];

			complete_max[j] = '\0';

			/* We put the string difference in a char*, because el_push()
			 * only accepts that type.
			 * It would have been simpler to just push the characters into
			 * the line buffer one after another.
			 */
			el_push(e, complete_max);

			puts("");
			for (i = 0; i < hits; i++) {
				printf("%s ", match[i]);
				free(match[i]);
			}
			puts("");
		break;
	}
	free(line_buf); line_buf = NULL;
	free(line_buf_copy); line_buf_copy = NULL;
	free(match); match = NULL;


	return(CC_REDISPLAY);
} /* el_tab_complete() */
#else
char *
cmd_generator(const char *text, int state)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*cname = NULL;

	command		*commands = commands_first;
	int		idx = 0;


	/* only search for a command name if this is not an already completed command
	 * (ie. there is no space in the line), OR if the already completed command is 'help'
	 */
	if (!strchr(rl_line_buffer, ' ')  ||  (strncmp(rl_line_buffer, "help ", 5) == 0))
		/* search for a command name */
		while (commands) {
			if (idx < state)
				idx++;
			else
				if (strncmp(text, commands->name, strlen(text)) == 0)
					return(strdup(commands->name));


			commands = commands->next;	/* iterate through the linked list */
		}

	/* only search for a keychain name if this is an already completed command
	 * (ie. there is space(s) in the line), AND it's not the 'help' command that
	 * has been completed previously
	 */
	if (strchr(rl_line_buffer, ' ')  &&  (strncmp(rl_line_buffer, "help ", 5) != 0)) {
		/* search for a keychain name */
		db_node = keychain->parent->children;
		while (db_node) {
			if (db_node->type == XML_ELEMENT_NODE) {	/* we only care about ELEMENT nodes */
				if (idx < state)
					idx++;
				else {
					cname = xmlGetProp(db_node, BAD_CAST "name");

					if (strncmp(text, (char *)cname, strlen(text)) == 0)
						return((char *)cname);

					xmlFree(cname); cname = NULL;
				}
			}

			db_node = db_node->next;
		}
	}

	return(NULL);
}
#endif


void
version(void)
{
	printf("%s %s\n", NAME, VERSION);
	puts("Compiled with "
#ifndef _READLINE
		"Editline"
#else
		"Readline"
#endif
#ifdef	_HAVE_PCRE
		", PCRE"
#endif
		" support.");
	puts("Written by LEVAI Daniel <leva@ecentrum.hu>");
	puts("Source, information, bugs: http://keychain.googlecode.com");
} /* help */


void
quit(int retval)
{
	if (getenv("KC_DEBUG"))
		puts("exiting...");

	if (bio_chain) {
		BIO_free_all(bio_chain);
		if (getenv("KC_DEBUG"))
			puts("closed bio_chain");
	}

	if (db_file) {
		if (close(db_file) == 0) {
			if (getenv("KC_DEBUG"))
				puts("closed database file");
		} else
			perror("close(database file)");
	}

#ifndef _READLINE
	if (eh) {
		history_end(eh);
		if (getenv("KC_DEBUG"))
			puts("closed editline history");
	}

	if (e) {
		el_end(e);
		if (getenv("KC_DEBUG"))
			puts("closed editline");
	}
#endif

	exit(retval);
} /* quit() */
