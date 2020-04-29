LOCALBASE ?=	/usr/local
BINDIR =	${LOCALBASE}/bin
PROG =		kc

MANDIR =	${LOCALBASE}/man/man
MAN =		kc.1

BCRYPT_DIR =	bcrypt

SRCS =		kc.c malloc_check.c
SRCS +=		cmd_c.c cmd_cdel.c cmd_clear.c cmd_clipboard.c cmd_clist.c cmd_cnew.c cmd_copy.c \
		cmd_cedit.c cmd_del.c cmd_edit.c cmd_export.c cmd_getnum.c \
		cmd_help.c cmd_import.c cmd_info.c cmd_list.c cmd_new.c cmd_near.c \
		cmd_passwd.c cmd_quit.c cmd_search.c cmd_searchre.c cmd_status.c \
		cmd_swap.c cmd_version.c cmd_write.c \
		commands.c commands_init.c ssha.c ykchalresp.c

CFLAGS +=	-pedantic -Wall
CFLAGS +=	`pkg-config --cflags libxml-2.0`
.ifdef READLINE
CFLAGS +=	-D_READLINE
.endif
.ifdef HAVE_PCRE
CFLAGS +=	`pkg-config --cflags libpcre` -D_HAVE_PCRE
.endif
.ifdef HAVE_LIBSCRYPT
CFLAGS +=	-D_HAVE_LIBSCRYPT
.endif
.ifdef BUNDLED_BCRYPT
CFLAGS +=	-D_BUNDLED_BCRYPT
.endif
.ifdef HAVE_YUBIKEY
CFLAGS +=	-D_HAVE_YUBIKEY `pkg-config --cflags ykpers-1`
.endif

LDADD +=	-lcrypto
.ifdef OS_OPENBSD
LDADD +=	-lutil
.endif
LDADD +=	`pkg-config --libs libxml-2.0`
.ifdef READLINE
LDADD +=	-lreadline -ltermcap
.else
LDADD +=	-ledit -lncursesw
.endif
.ifdef HAVE_PCRE
LDADD +=	`pkg-config --libs libpcre`
.endif
.ifdef HAVE_LIBSCRYPT
LDADD +=	-lscrypt
.endif
.ifdef HAVE_YUBIKEY
LDADD +=	-lyubikey `pkg-config --libs ykpers-1`
.endif

CLEANFILES +=	regress/test*

.ifdef BUNDLED_BCRYPT
all: bcrypt ${PROG}

.PHONY: bcrypt
bcrypt:
	cd ${BCRYPT_DIR}  &&  ${MAKE}

OBJS +=		${BCRYPT_DIR}/bcrypt_pbkdf.o ${BCRYPT_DIR}/blf.o ${BCRYPT_DIR}/explicit_bzero.o ${BCRYPT_DIR}/sha2.o
.endif

all: ${PROG}

test:
	sh regress/run_tests.sh

.include <bsd.prog.mk>
