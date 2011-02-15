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
#include <readpassphrase.h>
#else
#include <bsd/readpassphrase.h>
#endif

#include "common.h"
#include "commands.h"


void print_hex(unsigned char *, int);
char tab_complete(EditLine *, int);
void print_bio_chain(BIO *);


command		*commands_first = NULL;
xmlDocPtr	db = NULL;
xmlNodePtr	keychain = NULL;
char		dirty = 0;
char		*locale = NULL;


int main(int argc, char *argv[]) {

EditLine	*e = NULL;
int		e_count = 0;
const char	*e_line = NULL;
History		*eh = NULL;
HistEvent	eh_ev;

FILE		*db_file = NULL;
BIO		*bio_chain = NULL;
BIO		*bio_file = NULL;
BIO		*bio_cipher = NULL;
BIO		*bio_b64 = NULL;
char		*db_buf = NULL;
int		ret = 0;
size_t		db_buf_size = 4096, pos = 0;
unsigned char	key[128];
char		*pass = NULL;
const unsigned char	salt[17];
const unsigned char	iv[17];
void		*rbuf = NULL;
int		pass_maxlen = 64;
char		pass_prompt[15];
char		*pass_filename = NULL;
FILE		*pass_file = NULL;
size_t		pass_size = 128;

char		batchmode = 0;

struct stat	st;
const char	*default_db_dir = ".kc";
const char	*default_db_filename = "default";
char		*db_filename = NULL;
char		new_db_file = 0;
char		*env_home = NULL;

xmlNodePtr	db_root = NULL, db_node = NULL;

int		c = 0, len = 0;


	debug = 0;
	while ((c = getopt(argc, argv, "k:b:d")) != -1)
		switch (c) {
			case 'd':
				debug = 1;
			break;
			case 'k':
				db_filename = optarg;
			break;
			case 'b':
				batchmode = 1;
				pass_filename = optarg;
			break;
			default:
			break;
		}

	if (!db_filename) {
		// using default database

		env_home = getenv("HOME");

		len = strlen(env_home) + 1 + strlen(default_db_dir) + 1;
		db_filename = malloc(len); malloc_check(db_filename);

		// default db directory
		snprintf(db_filename, len, "%s/%s", env_home, default_db_dir);

		if(stat(db_filename, &st) == 0) {
			if(!S_ISDIR(st.st_mode)) {
				printf("'%s' is not a directory!\n", db_filename);
				quit(e, eh, bio_chain, EXIT_FAILURE);
			}
		} else {
			if (debug)
				printf("creating '%s'\n", db_filename);

			if(mkdir(db_filename, 0777) != 0) {
				printf("could not create '%s': %s\n", db_filename, strerror(errno));
				quit(e, eh, bio_chain, EXIT_FAILURE);
			}
		}

		// default db filename
		len += 1 + strlen(default_db_filename);
		db_filename = realloc(db_filename, len); malloc_check(db_filename);

		snprintf(db_filename, len, "%s/%s/%s", env_home, default_db_dir, default_db_filename);


		if(stat(db_filename, &st) == 0) {
			if(!S_ISLNK(st.st_mode)  &&  !S_ISREG(st.st_mode)) {
				printf("'%s' is not a regular file or a link!\n", db_filename);
				quit(e, eh, bio_chain, EXIT_FAILURE);
			}
			new_db_file = 0;

			printf("Opening '%s'\n", db_filename);

			// open the file and read the IV and the salt.
			db_file = fopen(db_filename, "r");

			rbuf = malloc(17); malloc_check(rbuf);

			fread(rbuf, 16, 1, db_file);
			strlcpy((char *)iv, (char *)rbuf, sizeof(iv));

			fread(rbuf, 16, 1, db_file);
			strlcpy((char *)salt, (char *)rbuf, sizeof(salt));

			fclose(db_file);

			strlcpy(pass_prompt, "Password: ", 15);
		} else {
			new_db_file = 1;

			printf("Creating '%s'\n", db_filename);

			// we'll write the IV and salt later after we got a password

			strlcpy(pass_prompt, "New password: ", 15);
		}
	}
	fflush(stdout);

	if (batchmode) {
		if (!pass_filename) {
			puts("You must specify a password file!");
			quit(e, eh, bio_chain, EXIT_FAILURE);
		}

		// read in the password from the specified file

		pass_file = fopen(pass_filename, "r");
		if (!pass_file) {
			perror("password file");
			quit(e, eh, bio_chain, EXIT_FAILURE);
		}

		pass = calloc(1, pass_size); malloc_check(pass);
		pos = 0;
		while (!feof(pass_file)) {
			// if we've reached the size of 'pass', grow it
			if (pos >= pass_size) {
				pass_size += 128;
				pass = realloc(pass, pass_size); malloc_check(pass);
			}

			pos += fread(pass, 1, pass_size, pass_file);
		}
		pass[pos] = '\0';
		if (strchr(pass, '\n'))
			pass[pos - 1] = '\0';		// strip the newline character
	} else {
		// ask for the password

		pass = malloc(pass_maxlen + 1); malloc_check(pass);
		readpassphrase(pass_prompt, pass, pass_maxlen + 1, RPP_ECHO_OFF | RPP_REQUIRE_TTY);
	}
	/*
	if (debug)
		printf("Password: '%s'\n", pass);
	*/

	// we'll write the IV and salt here, after we got a password,
	// so when the user interrupts the password prompt, we won't end up
	// with a messed up db file, which only contains the salt and the IV.
	if (new_db_file) {
		// create the new file and write the IV and the salt first.
		db_file = fopen(db_filename, "w");

		strlcpy((char *)iv, get_random(sizeof(iv) - 1), sizeof(iv));
		fwrite(iv, sizeof(iv) - 1, 1, db_file);

		strlcpy((char *)salt, get_random(sizeof(salt) - 1), sizeof(salt));
		fwrite(salt, sizeof(salt) - 1, 1, db_file);

		fclose(db_file);
	}

	/*
	if (debug)
		printf("iv='%s'\nsalt='%s'\n", iv, salt);
	*/


	bio_file = BIO_new_file(db_filename, "r+");
	if (!bio_file) {
		perror("BIO_new_file()");
		quit(e, eh, bio_chain, EXIT_FAILURE);
	}
	BIO_set_close(bio_file, BIO_CLOSE);
	bio_chain = BIO_push(bio_file, bio_chain);

	bio_b64 = BIO_new(BIO_f_base64());
	if (!bio_b64) {
		perror("BIO_new(f_base64)");
		quit(e, eh, bio_chain, EXIT_FAILURE);
	}
	bio_chain = BIO_push(bio_b64, bio_chain);

	bio_cipher = BIO_new(BIO_f_cipher());
	if (!bio_cipher) {
		perror("BIO_new(f_cipher)");
		quit(e, eh, bio_chain, EXIT_FAILURE);
	}
	bio_chain = BIO_push(bio_cipher, bio_chain);

	// generate a proper key for encoding/decoding BIO
	PKCS5_PBKDF2_HMAC_SHA1(pass, strlen(pass), salt, sizeof(salt), 5000, 128, key);// print_hex(key, 128);

	// turn on decoding
	BIO_set_cipher(bio_cipher, EVP_aes_256_cbc(), key, iv, 0);


	strlcpy(pass, "\0", pass_maxlen + 1);
	free(pass); pass = NULL;
	free(db_filename); db_filename = NULL;


	if (debug)
		print_bio_chain(bio_chain);


	// seek after the IV and salt (both 16 bytes)
	BIO_seek(bio_chain, 32);

	// read in the database file to a buffer
	db_buf = calloc(1, db_buf_size); malloc_check(db_buf);
	pos = 0;
	while(!BIO_eof(bio_chain)) {
		// if we've reached the size of 'db_buf', grow it
		if (pos >= db_buf_size) {
			db_buf_size += 4096;
			db_buf = realloc(db_buf, db_buf_size); malloc_check(db_buf);
		}

		ret = BIO_read(bio_chain, db_buf + pos, db_buf_size - pos);
		pos += ret;
		switch (ret) {
			case 0:
				//perror("read()");
			break;
			case -1:
				perror("read()");
				quit(e, eh, bio_chain, EXIT_FAILURE);
			break;
			case -2:
				if (debug)
					perror("unsupported operation");
			break;
			default:
			break;
		}
	}

	if (debug)
		printf("read %zd bytes\n", pos);


	if (!BIO_get_cipher_status(bio_chain)  &&  pos > 0) {
		puts("Failed to decrypt database file!");
		quit(e, eh, bio_chain, EXIT_FAILURE);
	}


	// turn on encoding
	BIO_set_cipher(bio_cipher, EVP_aes_256_cbc(), key, iv, 1);


	locale = getenv("LANG");
	if (locale)
		if (!xmlFindCharEncodingHandler(locale)) {
			if (debug)
				puts("falling back to default locale!");
			locale = "UTF-8";	// the default encoding
		}
	if (debug)
		printf("locale: %s\n", locale);



	if (pos == 0) {		// empty file?
		if (debug)
			puts("empty database file");

		// create a new document
		db = xmlNewDoc(BAD_CAST "1.0");
		if (!db) {
			puts("could not create XML document!");
			quit(e, eh, bio_chain, EXIT_FAILURE);
		}

		xmlCreateIntSubset(db, BAD_CAST "kc", NULL, BAD_CAST "kc.dtd");

		db_root = xmlNewNode(NULL, BAD_CAST "kc");	// 'THE' root node
		if (!db_root) {
			puts("could not create root node!");
			quit(e, eh, bio_chain, EXIT_FAILURE);
		}
		xmlDocSetRootElement(db, db_root);

		db_node = xmlNewText(BAD_CAST "\n  ");
		xmlAddChild(db_root, db_node);

		keychain = xmlNewNode(NULL, BAD_CAST "keychain");	// the first keychain ...
		if (!keychain) {
			puts("could not create default keychain!");
			quit(e, eh, bio_chain, EXIT_FAILURE);
		}
		xmlAddChild(db_root, keychain);
		xmlNewProp(keychain, BAD_CAST "name", BAD_CAST "default");	// ... its name is "default"

		db_node = xmlNewText(BAD_CAST "\n");
		xmlAddChild(db_root, db_node);

		// write the initial empty XML doc to the file
		cmd_write(e, e_line, eh, bio_chain);
	} else {
		if (debug)
			db = xmlReadMemory(db_buf, pos, NULL, "UTF-8", XML_PARSE_NONET);
		else
			db = xmlReadMemory(db_buf, pos, NULL, "UTF-8", XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
		if (!db) {
			puts("could not parse XML document!");
			quit(e, eh, bio_chain, EXIT_FAILURE);
		}

		db_root = xmlDocGetRootElement(db);
		if (!db_root) {
			puts("could not find root node!");
			quit(e, eh, bio_chain, EXIT_FAILURE);
		}
		if (!db_root->children) {
			puts("could not find first keychain!");
			quit(e, eh, bio_chain, EXIT_FAILURE);
		}

		keychain = db_root->children->next;
		if (!keychain) {
			puts("could not find first keychain!");
			quit(e, eh, bio_chain, EXIT_FAILURE);
		}
	}
	free(db_buf); db_buf = NULL;


	// init and start the CLI

	// init editline
	e = el_init("kc", stdin, stdout, stderr);
	if (!e) {
		perror("el_init()");
		quit(e, eh, bio_chain, EXIT_FAILURE);
	}

	// init editline history
	eh = history_init();
	if (!eh) {
		perror("history_init()");
		quit(e, eh, bio_chain, EXIT_FAILURE);
	}
	if (history(eh, &eh_ev, H_SETSIZE, 100) < 0) {
		fprintf(stderr, "history(H_SETSIZE): %s\n", eh_ev.str);
	}
	if (history(eh, &eh_ev, H_SETUNIQUE, 1) < 0) {
		fprintf(stderr, "history(H_SETUNIQUE): %s\n", eh_ev.str);
	}

	// setup editline/history parameters
	if (el_set(e, EL_CLIENTDATA, "") != 0) {
		perror("el_set(EL_CLIENTDATA)");
	}
	if (el_set(e, EL_PROMPT, e_prompt) != 0) {
		perror("el_set(EL_PROMPT)");
	}
	signal(SIGINT, SIG_IGN);
	if (el_set(e, EL_SIGNAL, 1) != 0) {
		perror("el_set(EL_SIGNAL)");
	}
	if (el_set(e, EL_HIST, history, eh) != 0) {
		perror("el_set(EL_HIST)");
	}
	if (el_set(e, EL_ADDFN, "TAB", "TAB completion", tab_complete) != 0) {
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
		quit(e, eh, bio_chain, EXIT_FAILURE);
	}


	// create the command list
	commands_init(&commands_first);


	// command loop
	do {
		e_line = el_gets(e, &e_count);

		if (e_count > 0) {
			if (strlen(e_line) > 1) {
				history(eh, &eh_ev, H_ENTER, e_line);
				cmd_match(e, e_line, eh, bio_chain);
			}
		}
	} while(1);


	return(0);
} /* main() */


void cmd_match(EditLine *e, const char *e_line, History *eh, BIO *bio_chain) {

int	idx = -1, space = -1;
char	*str = NULL, *line = strdup(e_line);
command	*commands = commands_first;


	line[strlen(line) - 1] = '\0';		// remove the newline character from the end


	// special case, if only a number was entered,
	// we display the appropriate entry, and if there
	// is another number after it, use it as space for jamming
	sscanf(line, "%d %d", &idx, &space);
	if (idx >= 0) {
		if (space >= 0)
			cmd_getnum(e, idx, space);
		else
			cmd_getnum(e, idx, 0);
	} else {
		// special case, if a '/'(slash) or 'c/' is the first character,
		// then everything that follows is a search pattern (even a space),
		// so we must not tokenize the line
		if (strncmp(line, "/", 1) == 0)
			str = "/";
		else
		if (strncmp(line, "c/", 2) == 0)
			str = "c/";
		else
			str = strtok(line, " ");

		while(commands) {
			if (strcmp(commands->name, str) == 0)	// find an exact match
				break;

			commands = commands->next;	// iterate through the linked list
		}

		if (commands)
			commands->fn(e, e_line, eh, bio_chain, commands);	// we call the command's respective function here
		else
			printf("unknown command: '%s'\n", line);
	}

	free(line); line = NULL;
} /* cmd_match() */


const char *e_prompt(EditLine *e) {

const char	*cl_data = NULL;
unsigned long	prompt_len = 0;
static char	*prompt = NULL;
xmlChar		*cname_locale = NULL, *cname = NULL;


	el_get(e, EL_CLIENTDATA, &cl_data);

	cname = xmlGetProp(keychain, BAD_CAST "name");
	cname_locale = convert_utf8(cname, 1);

	prompt_len = xmlStrlen(cname_locale) + 2 + strlen(cl_data) + 2 + 1;
	prompt = realloc(prompt, prompt_len); malloc_check(prompt);

	snprintf(prompt, prompt_len, "%s%% %s> ", cname_locale, cl_data);

	if (cname_locale)
		free(cname_locale);

	return(prompt);
} /* e_prompt() */


const char *e_prompt_null(EditLine *e) {
	return("");
}


void print_bio_chain(BIO *bio_chain) {
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


char *get_random_str(int length) {
int		i = 0;
char		*tmp = NULL;
char		*ret = NULL;


	ret = malloc(length + 1); malloc_check(ret);

	tmp = (char *)get_random(1);
	for (i=0; i < length; i++) {
		// until a printable character appears, read from random file,
		// plus ignore some characters I don't like :)
		while (	tmp[0] < 33  ||  tmp[0] > 125  ||
			tmp[0] == 34  || tmp[0] == 39  ||
			tmp[0] == 44  || tmp[0] == 46  ||
			tmp[0] == 96  || tmp[0] == 94) {

			if (tmp)
				free(tmp);

			tmp = (char *)get_random(1);
		}
		ret[i] = tmp[0];	// store the value
		tmp[0] = '\0';		// reset the value
	}

	if (tmp)
		free(tmp);

	ret[length] = '\0';


	return(ret);
} /* get_random_str() */


void *get_random(int length) {
char		*ret = NULL;
FILE		*rnd_file = NULL;


#ifndef _LINUX
	rnd_file = fopen("/dev/random", "r");
#else
	rnd_file = fopen("/dev/urandom", "r");
#endif
	if (!rnd_file) {
		puts("Error opening /dev/random!");
		return(NULL);
	}

	ret = malloc(length + 1); malloc_check(ret);

	fread(ret, length, 1, rnd_file);

	fclose(rnd_file);

	return(ret);
} /* get_random() */


char tab_complete(EditLine *e, int ch) {
char		*line_buf = NULL, *match = NULL, *tmp = NULL;
const LineInfo	*el_lineinfo = NULL;
command		*commands = commands_first;
int		hits = 0, len = 0;


	el_lineinfo = el_line(e);

	// copy the buffer (ie. line) to our variable 'line_buf',
	// because el_lineinfo->buffer is not NUL terminated
	len = el_lineinfo->lastchar - el_lineinfo->buffer;
	line_buf = malloc(len + 1); malloc_check(line_buf);
	memcpy(line_buf, el_lineinfo->buffer, len);
	line_buf[len] = '\0';


	// empty buffer (ie. line)
	if (strlen(line_buf) == 0)
		return(CC_CURSOR);


	len = 0;
	// initialize 'match' for use with strlcat()
	match = calloc(1, 1); malloc_check(match);
	while(commands) {
		tmp = strstr(commands->name, line_buf);
		if (tmp) {
			if (strcmp(tmp, commands->name) == 0) {		// this verifies that the match is not somewhere inside the command name, but it only matches from the beginning of the command name.
				hits++;

				len += strlen(commands->name) + 1 + 1;
				match = realloc(match, len); malloc_check(match);

				strlcat(match, commands->name, len);
				strlcat(match, " ", len);
			}
		}

		commands = commands->next;	// iterate through the linked list
	}

	switch (hits) {
		case 0:
		break;
		case 1:
			el_push(e, match + strlen(line_buf));	// print the commands remaining characters (remaining: the ones without the part (at the beginning) that we've entered already)
		break;
		default:
			printf("\n%s\n", match);	// more than one match
			el_set(e, EL_REFRESH);
		break;
	}
	if (line_buf)
		free(line_buf);
	if (match)
		free(match);


	return(CC_CURSOR);
} /* tab_complete() */


void quit(EditLine *e, History *eh, BIO *bio_chain, int retval) {
	if (bio_chain) {
		BIO_free_all(bio_chain);
		if (debug)
			printf("closed bio_chain\n");
	}

	if (eh) {
		history_end(eh);
		if (debug)
			printf("closed editline history\n");
	}

	if (e) {
		el_end(e);
		if (debug)
			printf("closed editline\n");
	}

	if (debug)
		printf("exiting...\n");

	exit(retval);
} /* quit() */


void print_hex(unsigned char *buf, int len) {
int i = 0;


	for (i = 0; i < len; i++) {
		printf("%02x", buf[i]);
	}
	puts("");
} /* print_hex() */
