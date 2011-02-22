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


#include "common.h"
#include "commands.h"


/* create a linked list for the commands.
 * the list contains the command's name and the function which handles it. */
void
commands_init(command **commands)
{
	command		*first = NULL;


	*commands = (command *)malloc(sizeof(command)); malloc_check(*commands);
	first = *commands;

	(*commands)->name = "list";
	(*commands)->usage = "list";
	(*commands)->help = "List all keys in the current keychain.";
	(*commands)->fn = cmd_list;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "new";
	(*commands)->usage = "new";
	(*commands)->help = "Add a new key to the current keychain.\n\nTo the value field, it is possible to enter multiline values\nby writing '\\n' in place of a needed new line.\nOne can escape this with '\\\\n'.";
	(*commands)->fn = cmd_new;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "edit";
	(*commands)->usage = "edit <index>";
	(*commands)->help = "Edit the given key in the current keychain.";
	(*commands)->fn = cmd_edit;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "copy";
	(*commands)->usage = "copy <index> <destination keychain's name or index>";
	(*commands)->help = "Copy the given key in the current keychain to the destination keychain.";
	(*commands)->fn = cmd_copy;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "del";
	(*commands)->usage = "del <index>";
	(*commands)->help = "Delete the given key from the current keychain.";
	(*commands)->fn = cmd_del;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "search";
	(*commands)->usage = "search <search string>";
	(*commands)->help = "Search for the given string in key names in the current keychain.";
	(*commands)->fn = cmd_search;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "/";
	(*commands)->usage = "/<search pattern>";
	(*commands)->help = "Search for the given regular expression in key names in the current keychain.";
	(*commands)->fn = cmd_searchre;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "clist";
	(*commands)->usage = "clist";
	(*commands)->help = "List all keychains.";
	(*commands)->fn = cmd_clist;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "c";
	(*commands)->usage = "c <keychain's name or index>";
	(*commands)->help = "Set the given keychain as the current keychain.";
	(*commands)->fn = cmd_c;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "cnew";
	(*commands)->usage = "cnew";
	(*commands)->help = "Add a new keychain.";
	(*commands)->fn = cmd_cnew;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "cren";
	(*commands)->usage = "cren <keychain's name or index>";
	(*commands)->help = "Rename the given keychain.";
	(*commands)->fn = cmd_cren;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "cdel";
	(*commands)->usage = "cdel <keychain's name or index>";
	(*commands)->help = "Delete the given keychain.";
	(*commands)->fn = cmd_cdel;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "csearch";
	(*commands)->usage = "csearch <search string>";
	(*commands)->help = "Search for the given string in keychain names.";
	(*commands)->fn = cmd_search;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "c/";
	(*commands)->usage = "c/<search pattern>";
	(*commands)->help = "Search for the given regular expression in keychain names.";
	(*commands)->fn = cmd_searchre;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "write";
	(*commands)->usage = "write";
	(*commands)->help = "Write out the current state of database to the database file.";
	(*commands)->fn = cmd_write;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "xport";
	(*commands)->usage = "xport <filename>";
	(*commands)->help = "Export the current state of database to the given file in UTF-8 encoded *clear text* XML format.";
	(*commands)->fn = cmd_export;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "import";
	(*commands)->usage = "import <filename>";
	(*commands)->help = "Import the specified UTF-8 encoded XML file, overwriting the currently used database.\nYou must 'write' the changes, if you want to save it to the currently opened database file.";
	(*commands)->fn = cmd_import;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "random";
	(*commands)->usage = "random [length]";
	(*commands)->help = "Generate a random alphanumeric string with the given length. Default is 8.";
	(*commands)->fn = cmd_random;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "clear";
	(*commands)->usage = "clear [count]";
	(*commands)->help = "Emulate a screen clearing.\nScrolls 50 lines by default, which can be multiplied by 'count' times if specified.";
	(*commands)->fn = cmd_clear;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "quit";
	(*commands)->usage = "quit";
	(*commands)->help = "Quit from program. Will ask if there are any unwritten changes.";
	(*commands)->fn = cmd_quit;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "version";
	(*commands)->usage = "version";
	(*commands)->help = "Display program version.";
	(*commands)->fn = cmd_version;
	(*commands)->next = (command *)malloc(sizeof(command)); malloc_check((*commands)->next);
	(*commands) = (*commands)->next;

	(*commands)->name = "help";
	(*commands)->usage = "help [command]";
	(*commands)->help = "Command list. If a command name is given as parameter, describe it.";
	(*commands)->fn = cmd_help;

	(*commands)->next = NULL;


	*commands = first;
} /* commands_init() */
