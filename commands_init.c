/*
 * Copyright (c) 2011-2023 LEVAI Daniel
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


#include "common.h"
#include "commands.h"


extern db_parameters	db_params;


/* create a linked list for the commands.
 * the list contains the command's name and the function which handles it. */
void
commands_init(command **commands)
{
	command		*first = NULL;


	*commands = (command *)malloc(sizeof(command)); malloc_check(*commands);
	first = *commands;

	if (!db_params.readonly) {
		(*commands)->name = "append";
		(*commands)->usage = "append -k <filename> [-A key type,key comment[,password]] "
#ifdef _HAVE_YUBIKEY
			"[-Y Key-slot,Device-index|Serial[,password]] "
#endif
			"[-P kdf] [-K key length] [-R kdf iterations] [-e cipher] [-m cipher mode] [-o]";
		(*commands)->help = "Append new and merge existing keychains to the database from a kc compatible encrypted database file named 'filename'. 'filename' must be a proper kc database. See command 'import' for description of parameters. Please consult the manual about the key limits and how kc applies them during appending. With the '-o' option you can import legacy (<v2.5) databases with missing attributes.\nSee commands 'appendxml', 'export' and 'import'.";
		(*commands)->fn = cmd_import;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "appendxml";
		(*commands)->usage = "appendxml -k <filename> [-o]";
		(*commands)->help = "Append new and merge existing keychains to the database from a kc compatible XML file named 'filename'. 'filename' must contain a properly formatted kc XML document. Please consult the manual about the key limits and how kc applies them during appending. With the '-o' option you can import legacy (<v2.5) databases with missing attributes.\nSee commands 'append', 'export' and 'import'.";
		(*commands)->fn = cmd_import;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "cdel";
		(*commands)->usage = "cdel <keychain>";
		(*commands)->help = "Delete a keychain. 'keychain' can be the keychain's index number or name. Index number takes priority when addressing a keychain.\nSee command 'ccdel'";
		(*commands)->fn = cmd_cdel;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "ccdel";
		(*commands)->usage = "ccdel <keychain name>";
		(*commands)->help = "Works like 'cdel', but the keychain's name takes priority over its index number.\nSee command 'cdel'";
		(*commands)->fn = cmd_cdel;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "cedit";
		(*commands)->usage = "cedit";
		(*commands)->help = "Edit the current keychain's name and description.";
		(*commands)->fn = cmd_cedit;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "cnew";
		(*commands)->usage = "cnew [name]";
		(*commands)->help = "Create a new keychain. If 'name' is not given then prompt for one.";
		(*commands)->fn = cmd_cnew;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "copy";
		(*commands)->usage = "copy <index> <keychain>";
		(*commands)->help = "Copy a key in the current keychain to another keychain. 'index' is the key's index to copy and 'keychain' is the destination keychain's index number or name. Index number takes priority when addressing a keychain.";
		(*commands)->fn = cmd_copy;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;
		(*commands)->name = "cp";
		(*commands)->usage = "cp <index> <keychain>";
		(*commands)->help = "Alias of 'copy'.";
		(*commands)->fn = cmd_copy;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "del";
		(*commands)->usage = "del <index>";
		(*commands)->help = "Delete a key from the current keychain. 'index' is the key's index number in the current keychain.";
		(*commands)->fn = cmd_del;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;
		(*commands)->name = "rm";
		(*commands)->usage = "rm <index>";
		(*commands)->help = "Alias of 'del'.";
		(*commands)->fn = cmd_del;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "edit";
		(*commands)->usage = "edit <index>";
		(*commands)->help = "Edit a key in the current keychain. 'index' is the key's index number in the current keychain.\n\nCharacter sequence rules in values apply to this command also.\nSee command 'new' for more information about this.";
		(*commands)->fn = cmd_edit;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "import";
		(*commands)->usage = "import -k <filename> [-A key type,key comment[,password]] "
#ifdef _HAVE_YUBIKEY
			"[-Y Key-slot,Device-index|Serial[,password]] "
#endif
			"[-P kdf] [-K key length] [-R kdf iterations] [-e cipher] [-m cipher mode] [-o]";
		(*commands)->help = "Import and overwrite the current database with the one from a kc compatible encrypted database file named 'filename'. 'filename' must be a proper kc database. Security key information, 'kdf', 'key length', 'KDF iterations', 'cipher' and 'cipher mode' can be used to specify these parameters if they differ from the current database. With the '-o' option you can import legacy (<v2.5) databases with missing attributes.\nSee commands 'importxml', 'export' and 'append'.";
		(*commands)->fn = cmd_import;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "importxml";
		(*commands)->usage = "importxml -k <filename> [-o]";
		(*commands)->help = "Import and overwrite the current database with the one from a kc compatible XML file named 'filename'. 'filename' must contain a properly formatted kc XML document. With the '-o' option you can import legacy (<v2.5) XML files with missing attributes.\nSee commands 'import', 'export' and 'append'.";
		(*commands)->fn = cmd_import;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "insert";
		(*commands)->usage = "insert <index> <index>";
		(*commands)->help = "Move the key at the first 'index' parameter to the index at the second 'index' parameter in the current keychain. Surrounding indices will be shifted backwards or forwards.";
		(*commands)->fn = cmd_swap;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "move";
		(*commands)->usage = "move <index> <keychain>";
		(*commands)->help = "Move a key in the current keychain to another keychain. 'index' is the key's index to move and 'keychain' is the destination keychain's index number or name. Index number takes priority when addressing a keychain.";
		(*commands)->fn = cmd_copy;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;
		(*commands)->name = "mv";
		(*commands)->usage = "mv <index> <keychain>";
		(*commands)->help = "Alias of 'move'.";
		(*commands)->fn = cmd_copy;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "new";
		(*commands)->usage = "new [name]";
		(*commands)->help = "Create a new key with a value in the current keychain. Both key and value will be prompted for, except when 'name' is specified; then it will be used as the key's name.\n\nCharacter sequences can be used in values:\n\"\\n\" - create a new line, and make the result a multiline value.\n\"\\r\", \"\\R\" - these will be replaced with 2 and 4 (respectively) random printable characters.\n\"\\a\", \"\\A\" - these will be replaced with 2 and 4 (respectively) random alpha-numeric characters.";
		(*commands)->fn = cmd_new;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "passwd";
		(*commands)->usage = "passwd [-A key type,key comment[,password]] "
#ifdef _HAVE_YUBIKEY
			"[-Y Key-slot,Device-index|Serial[,password]] "
#endif
			"[-P kdf] [-K key length] [-R kdf iterations] [-e cipher] [-m cipher mode]";
		(*commands)->help = "Change the database password or SSH public key identity being used to encrypt. Optionally, security key information, KDF, key length, KDF iterations, cipher and cipher mode can also be changed. All changes will be written immediately.";
		(*commands)->fn = cmd_passwd;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "swap";
		(*commands)->usage = "swap <index> <index>";
		(*commands)->help = "Swap two keys, exchanging their index numbers. The two 'index' parameters are the keys' index numbers in the current keychain.";
		(*commands)->fn = cmd_swap;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "write";
		(*commands)->usage = "write";
		(*commands)->help = "Save the database.";
		(*commands)->fn = cmd_write;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;
		(*commands)->name = "save";
		(*commands)->usage = "save";
		(*commands)->help = "Alias of 'write'.";
		(*commands)->fn = cmd_write;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;
	}

	(*commands)->name = "c";
	(*commands)->usage = "c <keychain>";
	(*commands)->help = "Change the current keychain. 'keychain' can be the keychain's index number or name. Index number takes priority when addressing a keychain.\nSee command 'cc'";
	(*commands)->fn = cmd_c;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "cc";
	(*commands)->usage = "cc <keychain name>";
	(*commands)->help = "Works like 'c', but the keychain's name takes priority over its index number.\nSee command 'c'";
	(*commands)->fn = cmd_c;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "c/";
	(*commands)->usage = "c/[modifier(s)] <pattern>";
	(*commands)->help = "Search for 'pattern' regular expression in keychain names.\nOptional modifiers:\n '!' suffix (eg.: c/!): show non-matching keychains.\n 'i' suffix (eg.: c/i): case of characters doesn't matter.\nYou can combine the modifiers.";
	(*commands)->fn = cmd_searchre;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "clear";
	(*commands)->usage = "clear [count]";
	(*commands)->help = "Emulate a screen clearing. Scrolls a 100 lines by default, which can be multiplied by 'count' times if specified.";
	(*commands)->fn = cmd_clear;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "clist";
	(*commands)->usage = "clist";
	(*commands)->help = "List keychains. Every keychain gets prefixed by its index number.";
	(*commands)->fn = cmd_clist;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;
	(*commands)->name = "cls";
	(*commands)->usage = "cls";
	(*commands)->help = "Alias of 'clist'.";
	(*commands)->fn = cmd_clist;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "csearch";
	(*commands)->usage = "csearch[modifier(s)] <string>";
	(*commands)->help = "Search for 'string' in keychain names.\nOptional modifiers:\n '!' suffix (eg.: csearch!): show non-matching keychains.\n 'i' suffix (eg.: csearchi): case of characters doesn't matter.\nYou can combine the modifiers.";
	(*commands)->fn = cmd_search;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "dump";
	(*commands)->usage = "dump -k <filename> [-c keychain]";
	(*commands)->help = "Dump the database to a kc compatible XML file named 'filename' (if no extension specified, \".xml\" will be appended). When specifying a keychain, dump only that keychain to the XML file. 'keychain' can be the keychain's index number or name. Index number takes priority when addressing a keychain.\nSee command 'export'\n\nNOTE: the created XML file will be plain text.";
	(*commands)->fn = cmd_export;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "export";
	(*commands)->usage = "export -k <filename> [-A key type,key comment[,password]] "
#ifdef _HAVE_YUBIKEY
		"[-Y Key-slot,Device-index|Serial[,password]] "
#endif
		"[-P kdf] [-K key length] [-R kdf iterations] [-e cipher] [-m cipher mode] [-c keychain]";
	(*commands)->help = "Export the database to a kc compatible encrypted database file named 'filename' (if no extension specified, \".kcd\" will be appended). When specifying 'keychain', export only that keychain. 'keychain' can be the keychain's index number or name. Index number takes priority when addressing a keychain. Security key information, 'kdf', 'key length', 'KDF iterations', 'cipher' and 'cipher mode' can be used to specify a different security key, KDF, key length, number of KDF iterations (if applicable), encryption cipher and cipher mode to use while exporting the database.\nSee commands 'dump', 'import' and 'append'.";
	(*commands)->fn = cmd_export;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "help";
	(*commands)->usage = "help [command]";
	(*commands)->help = "Print application help or describe a 'command'.";
	(*commands)->fn = cmd_help;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "info";
	(*commands)->usage = "info [index]";
	(*commands)->help = "Print information about a key in the current keychain or the keychain itself. If 'index' is specified, it is the key's index number in the current keychain. If omitted, information is about the current keychain.";
	(*commands)->fn = cmd_info;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "list";
	(*commands)->usage = "list [pager [offset]]";
	(*commands)->help = "List 'pager' number of keys per page from the current keychain, skipping 'offset' indices if specified. Every key gets prefixed by its index number. If 'pager' is not specified, the default value of 20 is used. The special value of 0 means to not use the pager. If 'offset' is not specified, it is not used.";
	(*commands)->fn = cmd_list;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;
	(*commands)->name = "ls";
	(*commands)->usage = "ls [pager [offset]]";
	(*commands)->help = "Alias of 'list'.";
	(*commands)->fn = cmd_list;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "near";
	(*commands)->usage = "near index [context]";
	(*commands)->help = "Display the keyname of key at 'index' position, and also print the surrounding keys' name in at most 'context' vicinity. Only the keys' names and index numbers get displayed.";
	(*commands)->fn = cmd_near;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "quit";
	(*commands)->usage = "quit";
	(*commands)->help = "Quit the program. If the database has been modified, then ask if it should be saved.";
	(*commands)->fn = cmd_quit;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;
	(*commands)->name = "exit";
	(*commands)->usage = "exit";
	(*commands)->help = "Alias of 'quit'.";
	(*commands)->fn = cmd_quit;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "search";
	(*commands)->usage = "search[modifier(s)] <string>";
	(*commands)->help = "Search for 'string' in key names in the current keychain.\nOptional modifiers:\n '!' suffix (eg.: search!): show non-matching keys.\n '*' suffix (eg.: search*): search in every keychain.\n 'i' suffix (eg.: searchi): case of characters doesn't matter.\nYou can combine the modifiers.";
	(*commands)->fn = cmd_search;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "status";
	(*commands)->usage = "status";
	(*commands)->help = "Display information about the database.";
	(*commands)->fn = cmd_status;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "tmux";
	(*commands)->usage = "tmux <index> [line]";
	(*commands)->help = "Copy the value of 'index' to tmux's paste buffer. 'index' is the key's index number in the current keychain. 'line' can be used to specify the line number to copy, if 'index' is a multiline value (defaults to 1).";
	(*commands)->fn = cmd_clipboard;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "version";
	(*commands)->usage = "version";
	(*commands)->help = "Display the program version.";
	(*commands)->fn = cmd_version;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "xclip";
	(*commands)->usage = "xclip <index> [line]";
	(*commands)->help = "Copy the value of 'index' to the PRIMARY X11 selection (ie.: middle mouse button). 'index' is the key's index number in the current keychain. 'line' can be used to specify the line number to copy, if 'index' is a multiline value (defaults to 1).";
	(*commands)->fn = cmd_clipboard;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "Xclip";
	(*commands)->usage = "Xclip <index> [line]";
	(*commands)->help = "Copy the value of 'index' to the CLIPBOARD X11 selection (aka.: CTRL+c - CTRL+v). 'index' is the key's index number in the current keychain. 'line' can be used to specify the line number to copy, if 'index' is a multiline value (defaults to 1).";
	(*commands)->fn = cmd_clipboard;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "/";
	(*commands)->usage = "/[modifier(s)] <pattern>";
	(*commands)->help = "Search for 'pattern' regular expression in key names in the current keychain.\nOptional modifiers:\n '!' suffix (eg.: /!): show non-matching keys.\n '*' suffix (eg.: /*): search in every keychain.\n 'i' suffix (eg.: /i): case of characters doesn't matter.\nYou can combine the modifiers.";
	(*commands)->fn = cmd_searchre;

	(*commands)->next = NULL;


	*commands = first;
} /* commands_init() */
