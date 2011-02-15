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


#include <stdarg.h>

#include "common.h"
#include "commands.h"


extern xmlNodePtr	keychain;
extern char		dirty;
extern char		*locale;


void cmd_new(EditLine *e, ...) {

va_list		ap;

History		*eh = NULL;

xmlNodePtr	db_node = NULL;
xmlChar		*key_locale = NULL, *value_locale = NULL, *key = NULL, *value = NULL;

int		e_count = 0;
const char	*e_line = NULL;


	va_start(ap, e);
	va_arg(ap, char *);	// ignore the (char *)line parameter
	eh = va_arg(ap, History *);
	va_end(ap);

	// disable history temporarily
	if (el_set(e, EL_HIST, history, NULL) != 0) {
		perror("el_set(EL_HIST)");
	}


	// set the new edit prompt
	if (el_set(e, EL_CLIENTDATA, "NEW key") != 0) {
		perror("el_set(EL_CLIENTDATA)");
	}

	e_line = el_gets(e, &e_count);
	if (!e_line) {
		perror("input");
		return;
	} else
		key_locale = BAD_CAST e_line;

	key_locale[xmlStrlen(key_locale) - 1] = '\0';	// remove the newline
	key = convert_utf8(key_locale, 0);


	// set the new edit prompt
	if (el_set(e, EL_CLIENTDATA, "NEW value") != 0) {
		perror("el_set(EL_CLIENTDATA)");
	}

	e_line = el_gets(e, &e_count);
	if (!e_line) {
		perror("input");
		if (key)
			free(key);
		return;
	} else
		value_locale = BAD_CAST e_line;

	value_locale[xmlStrlen(value_locale) - 1] = '\0';	// remove the newline
	value_locale = parse_newlines(value_locale, 0);
	value = convert_utf8(value_locale, 0);
	if (value_locale)
		free(value_locale);


	// change back to the default prompt
	if (el_set(e, EL_CLIENTDATA, "") != 0) {
		perror("el_set(EL_CLIENTDATA)");
	}
	// re-enable history
	if (el_set(e, EL_HIST, history, eh) != 0) {
		perror("el_set(EL_HIST)");
	}


	// XXX reloading a saved document inserts a 'text' element between each visible node (why?)
	// so we must reproduce this
	db_node = xmlNewText(BAD_CAST "\n    ");
	xmlAddChild(keychain, db_node);

	// add new element
	db_node = xmlNewTextChild(keychain, NULL, BAD_CAST "key", key);
	db_node = xmlNewTextChild(db_node, NULL, BAD_CAST "value", value);

	dirty = 1;

	if (key)
		free(key);
	if (value)
		free(value);
} /* cmd_new() */
