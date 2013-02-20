/*
 * Copyright (c) 2011, 2012, 2013 LEVAI Daniel
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


#include "common.h"
#include "commands.h"


extern unsigned char	readonly;


/* create a linked list for the commands.
 * the list contains the command's name and the function which handles it. */
void
commands_init(command **commands)
{
	command		*first = NULL;


	*commands = (command *)malloc(sizeof(command)); malloc_check(*commands);
	first = *commands;

	if (!readonly) {
		(*commands)->name = "append";
		(*commands)->usage = "append <filename>";
		(*commands)->help = "Append new and merge existing keychains to the database from the encrypted database file named 'filename'. 'filename' must be a proper kc database.\nSee command 'appendxml', 'xport' and 'import'.";
		(*commands)->fn = cmd_import;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "appendxml";
		(*commands)->usage = "appendxml <filename>";
		(*commands)->help = "Append new and merge existing keychains to the database from the XML file named 'filename'. 'filename' must contain a properly formatted kc XML document.\nSee command 'append', 'xport' and 'import'.";
		(*commands)->fn = cmd_import;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "cdel";
		(*commands)->usage = "cdel <keychain>";
		(*commands)->help = "Delete a keychain. 'keychain' can be the keychain's index number or name. Index number takes priority when addressing a keychain.";
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
		(*commands)->usage = "import <filename>";
		(*commands)->help = "Overwrite the current database with the one from the encrypted database file named 'filename'. 'filename' must be a proper kc database.\nSee command 'importxml', 'xport' and 'append'.";
		(*commands)->fn = cmd_import;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "importxml";
		(*commands)->usage = "importxml <filename>";
		(*commands)->help = "Overwrite the current database with the one from the XML file named 'filename'. 'filename' must contain a properly formatted kc XML document.\nSee command 'import', 'xport' and 'append'.";
		(*commands)->fn = cmd_import;
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
		(*commands)->help = "Create a new key with a value in the current keychain. Both key and value will be prompted for, except when 'name' is specified; then it will be used as the key's name.\n\nCharacter sequences can be used in values:\n\"\\n\" - create a new line, and make the result a multi-line value.\n\"\\r\", \"\\R\" - these will be replaced with 2 and 4 (respectively) random printable characters.\n\"\\a\", \"\\A\" - these will be replaced with 2 and 4 (respectively) random alpha-numeric characters.";
		(*commands)->fn = cmd_new;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "passwd";
		(*commands)->usage = "passwd";
		(*commands)->help = "Change the database password. All changes will be written immediately.";
		(*commands)->fn = cmd_passwd;
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
	(*commands)->help = "Change the current keychain. 'keychain' can be the keychain's index number or name. Index number takes priority when addressing a keychain.";
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
	(*commands)->help = "Search for 'pattern' regular expression in keychain names.\nOptional modifiers:\n '!' postfix (eg.: c/!): show non-matching keychains.\n 'i' postfix (eg.: c/i): case of characters doesn't matter.\nYou can combine the modifiers.";
	(*commands)->fn = cmd_searchre;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "clear";
	(*commands)->usage = "clear [count]";
	(*commands)->help = "Emulate a screen clearing. Scrolls 50 lines by default, which can be multiplied by 'count' times if specified.";
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
	(*commands)->help = "Search for 'string' in keychain names.\nOptional modifiers:\n '!' postfix (eg.: csearch!): show non-matching keychains.\n 'i' postfix (eg.: csearchi): case of characters doesn't matter.\nYou can combine the modifiers.";
	(*commands)->fn = cmd_search;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "dump";
	(*commands)->usage = "dump <filename> [keychain]";
	(*commands)->help = "Dump the database to the XML file named 'filename' (if no extension specified, \".xml\" will be appended). When specifying a keychain, dump only that keychain to the XML file. 'keychain' can be the keychain's index number or name. Index number takes priority when addressing a keychain.\nSee command 'xport'\n\nNOTE: the created XML file will be plain text.";
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
	(*commands)->usage = "list [pager]";
	(*commands)->help = "List 'pager' number of keys per page from current keychain. Every key gets prefixed by its index number. If 'pager' is not specified, the default value of 20 is used. The special value of 0 means to not use the pager.";
	(*commands)->fn = cmd_list;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;
	(*commands)->name = "ls";
	(*commands)->usage = "ls [keychain]";
	(*commands)->help = "Alias of 'list'.";
	(*commands)->fn = cmd_list;
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

	(*commands)->name = "random";
	(*commands)->usage = "random [length]";
	(*commands)->help = "Print a random string with 'length' length. The default 'length' is 8.";
	(*commands)->fn = cmd_random;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "search";
	(*commands)->usage = "search[modifier(s)] <string>";
	(*commands)->help = "Search for 'string' in key names in the current keychain.\nOptional modifiers:\n '!' postfix (eg.: search!): show non-matching keys.\n '*' postfix (eg.: search*): search in every keychain.\n 'i' postfix (eg.: searchi): case of characters doesn't matter.\nYou can combine the modifiers.";
	(*commands)->fn = cmd_search;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "status";
	(*commands)->usage = "status";
	(*commands)->help = "Display information about the database.";
	(*commands)->fn = cmd_status;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "version";
	(*commands)->usage = "version";
	(*commands)->help = "Display the program version.";
	(*commands)->fn = cmd_version;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "xport";
	(*commands)->usage = "xport <filename> [keychain]";
	(*commands)->help = "Export the database to the encrypted file named 'filename' (if no extension specified, \".kcd\" will be appended). When specifying a keychain, export only that keychain. 'keychain' can be the keychain's index number or name. Index number takes priority when addressing a keychain.\nSee command 'dump', 'import' and 'append'";
	(*commands)->fn = cmd_export;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "/";
	(*commands)->usage = "/[modifier(s)] <pattern>";
	(*commands)->help = "Search for 'pattern' regular expression in key names in the current keychain.\nOptional modifiers:\n '!' postfix (eg.: /!): show non-matching keys.\n '*' postfix (eg.: /*): search in every keychain.\n 'i' postfix (eg.: /i): case of characters doesn't matter.\nYou can combine the modifiers.";
	(*commands)->fn = cmd_searchre;

	(*commands)->next = NULL;


	*commands = first;
} /* commands_init() */
