/*
 * Copyright (c) 2011 LEVAI Daniel
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
 * DISCLAIMED. IN NO EVENT SHALL LEVAI Daniel BE LIABLE FOR ANY
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
#include <readpassphrase.h>
#else
#include <sys/file.h>
#include <bsd/readpassphrase.h>
#endif

#include "common.h"
#include "commands.h"


void print_bio_chain(BIO *);
#ifndef _READLINE
unsigned char el_tab_complete(EditLine *);
#else
char **rl_tab_complete(const char *, int, int);
char *cmd_generator(const char *, int);
#endif


BIO		*bio_chain = NULL;

command		*commands_first = NULL;

xmlDocPtr	db = NULL;
xmlNodePtr	keychain = NULL;

int		db_file = -1;

char		dirty = 0, batchmode = 0;

#ifndef _READLINE
	EditLine	*e = NULL;
	History		*eh = NULL;
	HistEvent	eh_ev;
#endif

char prompt_context[20];


int
main(int argc, char *argv[])
{
#ifndef _READLINE
	int		e_count = 0;
#endif
	const char	*e_line = NULL;
	char		*line = NULL;

	BIO		*bio_file = NULL;
	BIO		*bio_cipher = NULL;
	BIO		*bio_b64 = NULL;
	char		*db_buf = NULL;
	ssize_t		ret = -1;
	size_t		db_buf_size = 1024;
	unsigned int	pos = 0;
	unsigned char	key[128];
	char		*pass = NULL, *rand_str = NULL;
	unsigned char	salt[17], iv[17];
	void		*rbuf = NULL;
	size_t		pass_maxlen = 64;
	char		pass_prompt[15];
	char		*pass_filename = NULL;
	int		pass_file = -1;
	size_t		pass_size = 128;
	char		*cipher_mode = "cbc";

	struct stat	st;
	const char	*default_db_dir = ".kc";
	const char	*default_db_filename = "default";
	char		*db_filename = NULL;
	char		*env_home = NULL;

	xmlNodePtr	db_root = NULL;

	int		c = 0;
	size_t		len = 0;


	debug = 0;
	while ((c = getopt(argc, argv, "k:p:m:bvhd")) != -1)
		switch (c) {
			case 'k':
				db_filename = optarg;
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
				printf(	"%s %s\n"
					"-k: specify a non-default database file\n"
					"-p: read password from file.\n"
					"-m: cipher mode to use: cbc (default), cfb128, ofb.\n"
					"-b: batch mode: disable some features to enable commands from stdin.\n"
					"-v: version\n"
					"-h: this help\n"
					"-d: show some debug output\n", argv[0], USAGE);
				exit(EXIT_SUCCESS);
			break;
			case 'd':
				debug = 1;
			break;
			default:
				printf(	"%s %s\n", argv[0], USAGE);
				exit(EXIT_SUCCESS);
			break;
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
			if (debug)
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
		if (debug)
			puts("opening password file");

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
				pos += (unsigned int)ret;
		}
		pass[pos] = '\0';
		if (strrchr(pass, '\n'))
			pass[pos - 1] = '\0';		/* strip the newline character */

		if (close(pass_file) < 0)
			perror("close(password file)");
	} else {
		if(stat(db_filename, &st) == 0) /* if db_filename exists */
			strlcpy(pass_prompt, "Password: ", 15);
		else
			strlcpy(pass_prompt, "New password: ", 15);

		/* ask for the password */
		pass = malloc(pass_maxlen + 1); malloc_check(pass);
		readpassphrase(pass_prompt, pass, pass_maxlen + 1, RPP_ECHO_OFF | RPP_REQUIRE_TTY);
	}
	/*
	if (debug)
		printf("Password: '%s'\n", pass);
	*/

	if(stat(db_filename, &st) == 0) {	/* if db_filename exists */
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

		/* read the IV and the salt. */

		rbuf = malloc(17); malloc_check(rbuf);

		ret = read(db_file, rbuf, 16);
		if (ret < 0)
			perror("read(database file)");
		else
			strlcpy((char *)iv, (const char *)rbuf, sizeof(iv));

		ret = read(db_file, rbuf, 16);
		if (ret < 0)
			perror("read(database file)");
		else
			strlcpy((char *)salt, (const char *)rbuf, sizeof(salt));

		free(rbuf);
	} else {
		printf("Creating '%s'\n", db_filename);

		db_file = open(db_filename, O_RDWR | O_CREAT, 0600);
		if (db_file < 0) {
			perror("open(database file)");
			quit(EXIT_FAILURE);
		}

		if (debug)
			puts("generating salt and IV");

		/* generate the IV and the salt. */

		rand_str = get_random_str(sizeof(iv) - 1, 0);
		if (!rand_str) {
			puts("IV generation failure!");
			quit(EXIT_FAILURE);
		}
		strlcpy((char *)iv, rand_str, sizeof(iv));
		free(rand_str);

		rand_str = get_random_str(sizeof(salt) - 1, 0);
		if (!rand_str) {
			puts("Salt generation failure!");
			quit(EXIT_FAILURE);
		}
		strlcpy((char *)salt, rand_str, sizeof(salt));
		free(rand_str);

		if (debug)
			printf("iv='%s'\nsalt='%s'\n", iv, salt);

		write(db_file, iv, sizeof(iv) - 1);
		write(db_file, salt, sizeof(salt) - 1);
	}


	if (flock(db_file, LOCK_NB | LOCK_EX) < 0) {
		if (debug)
			puts("flock(database file)");

		puts("Could not lock the database file!\nMaybe another instance is using that database?");
		quit(EXIT_FAILURE);
	}


	bio_file = BIO_new_file(db_filename, "r+");
	if (!bio_file) {
		perror("BIO_new_file()");
		quit(EXIT_FAILURE);
	}
	BIO_set_close(bio_file, BIO_CLOSE);
	bio_chain = BIO_push(bio_file, bio_chain);

	bio_b64 = BIO_new(BIO_f_base64());
	if (!bio_b64) {
		perror("BIO_new(f_base64)");
		quit(EXIT_FAILURE);
	}
	bio_chain = BIO_push(bio_b64, bio_chain);

	bio_cipher = BIO_new(BIO_f_cipher());
	if (!bio_cipher) {
		perror("BIO_new(f_cipher)");
		quit(EXIT_FAILURE);
	}
	bio_chain = BIO_push(bio_cipher, bio_chain);

	/* generate a proper key for encoding/decoding BIO */
	PKCS5_PBKDF2_HMAC_SHA1(pass, (int)strlen(pass), salt, sizeof(salt), 5000, 128, key);

	/* turn on decoding */
	if (strcmp(cipher_mode, "cfb128") == 0) {
		if (debug)
			printf("using cipher mode: %s\n", cipher_mode);
		BIO_set_cipher(bio_cipher, EVP_aes_256_cfb128(), key, iv, 0);
	} else if (strcmp(cipher_mode, "ofb") == 0) {
		if (debug)
			printf("using cipher mode: %s\n", cipher_mode);
		BIO_set_cipher(bio_cipher, EVP_aes_256_ofb(), key, iv, 0);
	} else {	/* the default is CBC */
		if (debug)
			printf("using default cipher mode: %s\n", cipher_mode);
		BIO_set_cipher(bio_cipher, EVP_aes_256_cbc(), key, iv, 0);
	}


	memset(pass, '\0', pass_maxlen);
	free(pass); pass = NULL;


	if (debug)
		print_bio_chain(bio_chain);


	/* seek after the IV and salt (both 16 bytes) */
	BIO_seek(bio_chain, 32);

	/* read in the database file to a buffer */
	db_buf = calloc(1, db_buf_size); malloc_check(db_buf);
	pos = 0;
	do {
		/* if we've reached the size of 'db_buf', grow it */
		if (db_buf_size <= pos) {
			db_buf_size += 1024;
			db_buf = realloc(db_buf, db_buf_size); malloc_check(db_buf);
		}

		ret = BIO_read(bio_chain, db_buf + pos, (int)(db_buf_size - pos));
		if (debug)
			printf("BIO_read(): %d\n", (unsigned int)ret);
		switch (ret) {
			case 0:
				if (BIO_should_retry(bio_chain)) {
					if (debug)
						puts("read delay");

					sleep(1);
					continue;
				}
			break;
			case -1:
				if (BIO_should_retry(bio_chain)) {
					if (debug)
						puts("read delay");

					sleep(1);
					continue;
				} else {
					if (debug)
						perror("BIO_read() error (don't retry)");

					puts("There was an error while trying to read the database!");
				}
			break;
			case -2:
				if (debug)
					perror("unsupported operation");

				puts("There was an error while trying to read the database!");
			break;
			default:
				pos += (unsigned int)ret;
				if (debug)
					printf("pos: %d\n", pos);
			break;
		}
	} while (ret > 0);

	if (debug)
		printf("read %d bytes\n", pos);


	if (BIO_get_cipher_status(bio_chain) == 0  &&  pos > 0) {
		puts("Failed to decrypt database file!");
		quit(EXIT_FAILURE);
	}


	/* turn on encoding */
	if (strcmp(cipher_mode, "cfb128") == 0) {
		if (debug)
			printf("using cipher mode: %s\n", cipher_mode);
		BIO_set_cipher(bio_cipher, EVP_aes_256_cfb128(), key, iv, 1);
	} else if (strcmp(cipher_mode, "ofb") == 0) {
		if (debug)
			printf("using cipher mode: %s\n", cipher_mode);
		BIO_set_cipher(bio_cipher, EVP_aes_256_ofb(), key, iv, 1);
	} else {	/* the default is CBC */
		if (debug)
			printf("using default cipher mode: %s\n", cipher_mode);
		BIO_set_cipher(bio_cipher, EVP_aes_256_cbc(), key, iv, 1);
	}


	if (pos == 0) {		/* empty file? */
		if (debug)
			puts("empty database file");

		/* create a new document */
		db = xmlNewDoc(BAD_CAST "1.0");
		if (!db) {
			puts("Could not create XML document!");
			quit(EXIT_FAILURE);
		}

		xmlCreateIntSubset(db, BAD_CAST "kc", NULL, BAD_CAST "kc.dtd");

		db_root = xmlNewNode(NULL, BAD_CAST "kc");	/* 'THE' root node */
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

		xmlAddChild(keychain, xmlNewText(BAD_CAST "\n\t"));

		xmlAddChild(db_root, xmlNewText(BAD_CAST "\n"));

		/* write the initial empty XML doc to the file */
		cmd_write(NULL, NULL);
	} else {
		if (debug)
			db = xmlReadMemory(db_buf, (int)pos, NULL, "UTF-8", XML_PARSE_NONET | XML_PARSE_RECOVER);
		else
			db = xmlReadMemory(db_buf, (int)pos, NULL, "UTF-8", XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_RECOVER);
		if (!db) {
			puts("Could not parse XML document!");
			quit(EXIT_FAILURE);
		}

		db_root = xmlDocGetRootElement(db);
		if (!db_root) {
			puts("Could not find root node!");
			puts("If you have specified 'cfb128' or 'ofb' for chipher mode, this could\n"
				"also mean that either you have entered a wrong password for the\n"
				"database or specified a cipher mode other than the database was\n"
				"encrypted with!");
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
	}
	free(db_buf); db_buf = NULL;


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
	if (history(eh, &eh_ev, H_SETSIZE, 100) < 0) {
		fprintf(stderr, "history(H_SETSIZE): %s\n", eh_ev.str);
	}
	if (history(eh, &eh_ev, H_SETUNIQUE, 1) < 0) {
		fprintf(stderr, "history(H_SETUNIQUE): %s\n", eh_ev.str);
	}

	/* setup editline/history parameters */
	if (el_set(e, EL_PROMPT, prompt_str) != 0) {
		perror("el_set(EL_PROMPT)");
	}
	if (el_set(e, EL_SIGNAL, 1) != 0) {
		perror("el_set(EL_SIGNAL)");
	}
	if (el_set(e, EL_HIST, history, eh) != 0) {
		perror("el_set(EL_HIST)");
	}
	if (el_set(e, EL_ADDFN, "TAB", "TAB completion", el_tab_complete) != 0) {
		perror("el_set(EL_ADDFN)");
	}
	if (el_set(e, EL_BIND, "\t", "TAB", NULL) != 0) {
		perror("el_set(EL_BIND)");
	}
	if (el_set(e, EL_BIND, "^R", "ed-redisplay", NULL) != 0) {
		perror("el_set(EL_BIND)");
	}
	if (el_set(e, EL_BIND, "^E", "ed-move-to-end", NULL) != 0) {
		perror("el_set(EL_BIND)");
	}
	if (el_set(e, EL_BIND, "^A", "ed-move-to-beg", NULL) != 0) {
		perror("el_set(EL_BIND)");
	}
	if (el_source(e, NULL) != 0) {
		if (errno != 0) {
			if (errno != ENOENT)
				perror("Error executing .editrc");
		} else {
			printf("Error executing .editrc\n");
		}
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

	rl_attempted_completion_function = rl_tab_complete;
#endif

	strlcpy(prompt_context, "", sizeof(prompt_context));


	/* create the command list */
	commands_init(&commands_first);


	/* command loop */
	do {
#ifndef _READLINE
		e_line = el_gets(e, &e_count);
#else
		e_line = readline(prompt_str());
#endif
		if (e_line)
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
	} while (e_line);

	cmd_quit(NULL, NULL);

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
		/*
		 * special case, if a '[*]/'([*]slash) or 'c/' is the first character,
		 * then everything that follows is a search pattern (even a space),
		 * so we must not tokenize the line
		 */
		if (strncmp(line, "/", 1) == 0)
			cmd = "/";
		else if (strncmp(line, "*/", 2) == 0)
			cmd = "*/";
		else if (strncmp(line, "c/", 2) == 0)
			cmd = "c/";
		else {
			cmd = strtok(line, " ");
			if (!cmd)	/* probably an empty line */
				return;
		}

		while(commands) {
			if (strcmp(commands->name, cmd) == 0)	/* find an exact match */
				break;

			commands = commands->next;	/* iterate through the linked list */
		}

		if (commands)
			commands->fn(e_line, commands);	/* we call the command's respective function here */
		else
			printf("unknown command: '%s'\n", cmd);
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

	prompt_len = (size_t)xmlStrlen(cname) + 2 + sizeof(prompt_context) + 2 + 1;
	prompt = realloc(prompt, prompt_len); malloc_check(prompt);

	snprintf(prompt, prompt_len, "%s%% %s> ", cname, prompt_context);
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


char *
get_random_str(size_t length, char alnum)
{
	int		i = 0;
	int		rnd_file = -1;
#ifndef _LINUX
	char		*rnd_dev = "/dev/random";
#else
	char		*rnd_dev = "/dev/urandom";
#endif
	char		*tmp = NULL;
	ssize_t		ret = -1;
	char		*rnd_str = NULL;


	rnd_file = open(rnd_dev, O_RDONLY);
	if (rnd_file < 0) {
		printf("Error opening %s!", rnd_dev);
		perror("open()");
		return(NULL);
	}


	rnd_str = malloc((size_t)length + 1); malloc_check(rnd_str);
	tmp = malloc(1); malloc_check(tmp);

	read(rnd_file, tmp, 1);
	for (i=0; i < (int)length; i++) {
		if (alnum)	/* only alphanumeric was requested */
			while (	(*tmp < 65  ||  *tmp > 90)  &&
				(*tmp < 97  ||  *tmp > 122)  &&
				(*tmp < 48  ||  *tmp > 57)) {

				ret = read(rnd_file, tmp, 1);
				if (ret < 0) {
					perror("read(random device)");
					return(NULL);
				}
			}
		else
			/* give anything printable */
			while (	*tmp < 33  ||  *tmp > 126) {

				ret = read(rnd_file, tmp, 1);
				if (ret < 0) {
					perror("read(random device)");
					return(NULL);
				}
			}

		rnd_str[i] = *tmp;		/* store the value */
		*tmp = '\0';		/* reset the value */
	}

	free(tmp); tmp = NULL;

	rnd_str[(long)length] = '\0';

	close(rnd_file);


	return(rnd_str);
} /* get_random_str() */


#ifndef _READLINE
unsigned char
el_tab_complete(EditLine *e)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*cname = NULL;

	char		*line_buf = NULL, *line_buf_copy = NULL, *match = NULL, *word_next = NULL, *word = NULL;
	const LineInfo	*el_lineinfo = NULL;
	command		*commands = commands_first;
	int		hits = 0;
	size_t		match_len = 0, word_len = 0;
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

	/* empty buffer (ie. line) */
	if (!line_buf_len)
		return(CC_CURSOR);


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
	/* nothing in the 'line_buf', other than space(s) */
	if (!word)
		return(CC_CURSOR);

	word_len = strlen(word);


	/* initialize 'match' for use with strlcat() */
	match = calloc(1, 1); malloc_check(match);

	/*
	 * Basically, the first word will only be completed to a command.
	 * The second (or any additional) word will be completed to a command only if the first word was 'help' (because help [command] shows the help for command),
	 * or (if the first word is something other than 'help') it will be completed to a keychain name (to serve as a parameter for the previously completed command).
	 */
	if (!strchr(line_buf, ' ')  ||  (strncmp(line_buf, "help ", 5) == 0))	/* only search for a command name if this is not an already completed command (ie. there is no space in the line), OR if the already completed command is 'help' */
		/* search for a command name */
		do {
			if (strncmp(word, commands->name, word_len) == 0) {
				hits++;

				match_len += strlen(commands->name) + 1 + 1;
				match = realloc(match, match_len); malloc_check(match);

				if (hits > 1)	/* don't prefix the first match with a space */
					strlcat(match, " ", match_len);

				strlcat(match, commands->name, match_len);
			}
		} while((commands = commands->next));	/* iterate through the linked list */

	if (strchr(line_buf, ' ')  &&  (strncmp(line_buf, "help ", 5) != 0)) {	/* only search for a keychain name if this is an already completed command (ie. there is space(s) in the line), AND it's not the 'help' command that has been completed previously */
		/* search for a keychain name */
		db_node = keychain->parent->children;
		while (db_node) {
			if (db_node->type == XML_ELEMENT_NODE) {	/* we only care about ELEMENT nodes */
				cname = xmlGetProp(db_node, BAD_CAST "name");

				if (strncmp(word, (char *)cname, word_len) == 0) {
					hits++;

					match_len += strlen((char *)cname) + 1 + 1;
					match = realloc(match, match_len); malloc_check(match);

					if (hits > 1)	/* don't prefix the first match with a space */
						strlcat(match, " ", match_len);

					strlcat(match, (char *)cname, match_len);
				}

				xmlFree(cname); cname = NULL;
			}

			db_node = db_node->next;
		}
	}


	switch (hits) {
		case 0:
			printf("\a");			/* no match */
		break;
		case 1:
			el_push(e, match + (int)word_len);	/* print the word's remaining characters (remaining: the ones without the part (at the beginning) that we've entered already) */
			if (line_buf[line_buf_len - 1] != ' ')	/* if this is not an already completed word (ie. there is no space at the end), then ... */
				el_push(e, " ");		/* ... put a space after the completed word */
		break;
		default:
			printf("\n%s\n", match);	/* more than one match */
			el_set(e, EL_REFRESH);
		break;
	}
	free(line_buf);
	free(line_buf_copy);
	free(match);


	return(CC_REDISPLAY);
} /* el_tab_complete() */
#else
char **
rl_tab_complete(const char *text, int start, int end)
{
	char	**matches = NULL;


	matches = rl_completion_matches(text, cmd_generator);

	return(matches);
} /* rl_tab_complete() */

char *
cmd_generator(const char *text, int state)
{
	xmlNodePtr	db_node = NULL;
	xmlChar		*cname = NULL;

	command		*commands = commands_first;
	int		idx = 0;


	/* search for a command name */
	while (commands) {
		if (idx < state)
			idx++;
		else
			if (strncmp(text, commands->name, strlen(text)) == 0)
				return(strdup(commands->name));


		commands = commands->next;	/* iterate through the linked list */
	}

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

	return(NULL);
}
#endif


void
version(void)
{
	printf("%s %s\n", NAME, VERSION);
	printf("%s was written by Daniel LEVAI <leva@ecentrum.hu>\n"
		"Source, information, bugs:\n"
		"http://keychain.googlecode.com\n", NAME);
} /* help */

void
quit(int retval)
{
	if (bio_chain) {
		BIO_free_all(bio_chain);
		if (debug)
			puts("closed bio_chain");
	}

	if (db_file) {
		close(db_file);
		if (debug)
			puts("closed database file");
	}

	if (debug)
		puts("exiting...");

#ifndef _READLINE
	if (eh) {
		history_end(eh);
		if (debug)
			puts("closed editline history");
	}

	if (e) {
		el_end(e);
		if (debug)
			puts("closed editline");
	}
#endif

	exit(retval);
} /* quit() */
