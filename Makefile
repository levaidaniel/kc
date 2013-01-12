LOCALBASE ?=	/usr/local
BINDIR =	${LOCALBASE}/bin
PROG =		kc

MANDIR =	${LOCALBASE}/man/man
MAN =		kc.1

SRCS =		kc.c malloc_check.c
SRCS +=		cmd_c.c cmd_cdel.c cmd_clear.c cmd_clist.c cmd_cnew.c cmd_copy.c \
		cmd_cren.c cmd_del.c cmd_edit.c cmd_export.c cmd_getnum.c \
		cmd_help.c cmd_import.c cmd_info.c cmd_list.c cmd_new.c cmd_passwd.c \
		cmd_quit.c cmd_random.c cmd_search.c cmd_searchre.c cmd_status.c \
		cmd_version.c cmd_write.c \
		commands.c commands_init.c

.ifdef READLINE
CFLAGS +=	-D_READLINE
.endif
CFLAGS +=	-pedantic -Wall -g
CFLAGS +=	`pkg-config --cflags libxml-2.0`
.ifdef HAVE_PCRE
CFLAGS +=	`pkg-config --cflags libpcre` -D_HAVE_PCRE
.endif

LDADD +=	-lssl -lcrypto
.ifdef READLINE
LDADD +=	-lreadline -ltermcap
.else
LDADD +=	-ledit -lncursesw
.endif
LDADD +=	`pkg-config --libs libxml-2.0`
.ifdef HAVE_PCRE
LDADD +=	`pkg-config --libs libpcre`
.endif

CLEANFILES +=	regress/test*

test:
	sh regress/run_tests.sh

.include <bsd.prog.mk>
