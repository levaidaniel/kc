/*
 * Copyright (c) 2011, 2012 LEVAI Daniel
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

	if (readonly == 0) {
		(*commands)->name = "new";
		(*commands)->usage = "new [name]";
		(*commands)->help = "Create a new key in the current keychain. If 'name' is specified it\nwill be the key's name, otherwise prompt for that too. It is\npossible to enter multiline values by writing '\\n' in place of a\ndesired new line. One can escape this with '\\\\n'.";
		(*commands)->fn = cmd_new;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "edit";
		(*commands)->usage = "edit <index>";
		(*commands)->help = "Edit a key in the current keychain. 'index' is the key's index\nnumber in the current keychain.";
		(*commands)->fn = cmd_edit;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "copy";
		(*commands)->usage = "copy <index> <keychain>";
		(*commands)->help = "Copy a key in the current keychain to another keychain. 'index' is\nthe key's index to copy and 'keychain' is the destination keychain's\nindex number or name.";
		(*commands)->fn = cmd_copy;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;
		(*commands)->name = "cp";
		(*commands)->usage = "cp <index> <keychain>";
		(*commands)->help = "Alias of 'copy'.\nCopy a key in the current keychain to another keychain. 'index' is\nthe key's index to copy and 'keychain' is the destination keychain's\nindex number or name.";
		(*commands)->fn = cmd_copy;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "move";
		(*commands)->usage = "move <index> <keychain>";
		(*commands)->help = "Move a key in the current keychain to another keychain. 'index' is\nthe key's index to move and 'keychain' is the destination keychain's\nindex number or name.";
		(*commands)->fn = cmd_copy;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;
		(*commands)->name = "mv";
		(*commands)->usage = "mv <index> <keychain>";
		(*commands)->help = "Alias of 'move'.\nMove a key in the current keychain to another keychain. 'index' is\nthe key's index to move and 'keychain' is the destination keychain's\nindex number or name.";
		(*commands)->fn = cmd_copy;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "del";
		(*commands)->usage = "del <index>";
		(*commands)->help = "Delete a key from the current keychain. 'index' is the key's index\nnumber in the current keychain.";
		(*commands)->fn = cmd_del;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;
		(*commands)->name = "rm";
		(*commands)->usage = "rm <index>";
		(*commands)->help = "Alias of 'del'.\nDelete a key from the current keychain. 'index' is the key's index\nnumber in the current keychain.";
		(*commands)->fn = cmd_del;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "cnew";
		(*commands)->usage = "cnew [name]";
		(*commands)->help = "Create a new keychain. If 'name' is not given then prompt for one.";
		(*commands)->fn = cmd_cnew;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "cren";
		(*commands)->usage = "cren <keychain>";
		(*commands)->help = "Rename a keychain. 'keychain' can be an index number or the\nkeychain's name.";
		(*commands)->fn = cmd_cren;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "cdel";
		(*commands)->usage = "cdel <keychain>";
		(*commands)->help = "Delete a keychain. 'keychain' can be an index number or the\nkeychain's name.";
		(*commands)->fn = cmd_cdel;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "write";
		(*commands)->usage = "write";
		(*commands)->help = "Save the current database.";
		(*commands)->fn = cmd_write;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;
		(*commands)->name = "save";
		(*commands)->usage = "save";
		(*commands)->help = "Alias of 'write'.\nSave the current database.";
		(*commands)->fn = cmd_write;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;

		(*commands)->name = "import";
		(*commands)->usage = "import <filename>";
		(*commands)->help = "Import a database from the XML file named 'filename' It must be a\nproperly exported XML document. NOTE that the current database\nwill be overwritten if saved.";
		(*commands)->fn = cmd_import;
		(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
		(*commands) = (*commands)->next;
	}

	(*commands)->name = "list";
	(*commands)->usage = "list [keychain]";
	(*commands)->help = "List the keys in the current keychain or if specified, in the\nkeychain named 'keychain'. Every key gets prefixed by its index\nnumber. 'keychain' can be an index number or the keychain's name.";
	(*commands)->fn = cmd_list;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;
	(*commands)->name = "ls";
	(*commands)->usage = "ls [keychain]";
	(*commands)->help = "Alias of 'list'.\nList the keys in the current keychain or if specified, in the\nkeychain named 'keychain'. Every key gets prefixed by its index\nnumber. 'keychain' can be an index number or the keychain's name.";
	(*commands)->fn = cmd_list;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "search";
	(*commands)->usage = "search <string>";
	(*commands)->help = "Search for 'string' in key names in the current keychain. If the\nsearch command is prefixed by a '*' (eg.: *search) then search in\nevery keychain in the current database.";
	(*commands)->fn = cmd_search;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "*search";
	(*commands)->usage = "*search <string>";
	(*commands)->help = "Search for 'string' in key names in every keychain.";
	(*commands)->fn = cmd_search;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "/";
	(*commands)->usage = "/<pattern>";
	(*commands)->help = "Search for 'pattern' regular expression in key names in the current\nkeychain. If the / command is prefixed by a '*' (eg.: */) then\nsearch in every keychain in the current database.";
	(*commands)->fn = cmd_searchre;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "*/";
	(*commands)->usage = "*/<pattern>";
	(*commands)->help = "Search for 'pattern' in key names in every keychain.";
	(*commands)->fn = cmd_searchre;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "clist";
	(*commands)->usage = "clist";
	(*commands)->help = "List keychains in the current database. Every keychain gets\nprefixed by its index number.";
	(*commands)->fn = cmd_clist;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "c";
	(*commands)->usage = "c <keychain>";
	(*commands)->help = "Change the current keychain. 'keychain' can be an index number or\nthe keychain's name.";
	(*commands)->fn = cmd_c;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "csearch";
	(*commands)->usage = "csearch <string>";
	(*commands)->help = "Search for 'string' in keychain names in the current database.";
	(*commands)->fn = cmd_search;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "c/";
	(*commands)->usage = "c/<pattern>";
	(*commands)->help = "Search for 'pattern' regular expression in keychain names in the\ncurrent database.";
	(*commands)->fn = cmd_searchre;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "xport";
	(*commands)->usage = "xport <filename>";
	(*commands)->help = "Export the current database to the XML file named 'filename'.";
	(*commands)->fn = cmd_export;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "random";
	(*commands)->usage = "random [length]";
	(*commands)->help = "Print a random string with 'length' length. The default 'length' is\n8.";
	(*commands)->fn = cmd_random;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "clear";
	(*commands)->usage = "clear [count]";
	(*commands)->help = "Emulate a screen clearing. Scrolls 50 lines by default, which can\nbe multiplied by 'count' times if specified.";
	(*commands)->fn = cmd_clear;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "quit";
	(*commands)->usage = "quit";
	(*commands)->help = "Quit the program. If the database is modified, then ask if it\nshould be saved.";
	(*commands)->fn = cmd_quit;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;
	(*commands)->name = "exit";
	(*commands)->usage = "exit";
	(*commands)->help = "Alias of 'quit'.\nQuit the program. If the database is modified, then ask if it\nshould be saved.";
	(*commands)->fn = cmd_quit;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "version";
	(*commands)->usage = "version";
	(*commands)->help = "Display the program version.";
	(*commands)->fn = cmd_version;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "help";
	(*commands)->usage = "help [command]";
	(*commands)->help = "Print application help or describe a 'command'.";
	(*commands)->fn = cmd_help;

	(*commands)->next = NULL;


	*commands = first;
} /* commands_init() */
