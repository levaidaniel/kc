PROG =		kc

SRCS =		kc.c malloc_check.c
SRCS +=		cmd_c.c cmd_cdel.c cmd_clist.c cmd_cnew.c cmd_cren.c cmd_del.c \
		cmd_edit.c cmd_export.c cmd_import.c cmd_getnum.c cmd_help.c \
		cmd_list.c cmd_new.c cmd_quit.c cmd_random.c cmd_search.c \
		cmd_searchre.c cmd_write.c cmd_version.c commands.c commands_init.c

CFLAGS +=	-Wall -g
CFLAGS +=	`pkg-config --cflags libxml-2.0`
.ifdef HAVE_PCRE
CFLAGS +=	`pkg-config --cflags libpcre` -D_HAVE_PCRE
.endif

LDADD +=	-lssl -lcrypto
LDADD +=	-ledit -ltermcap
LDADD +=	`pkg-config --libs libxml-2.0`
.ifdef HAVE_PCRE
LDADD +=	`pkg-config --libs libpcre`
.endif


CLEANFILES +=	*.cat[0-9]


all: ${PROG}

${PROG}: ${SRCS}
	${CC} -o ${PROG} ${CFLAGS} ${LDADD} \
		${SRCS}

.include <bsd.prog.mk>
