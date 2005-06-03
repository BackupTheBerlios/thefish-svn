# $Id$
# This is a BSD Makefile, not a GNUMakefile!
#
LOCALBASE?=/usr/local

.if !defined(NO_GUI)
GTK_CFLAGS!=pkg-config gtk+-2.0 --cflags
CFLAGS+= -Wall -g -DGTK_DISABLE_DEPRECATED ${GTK_CFLAGS}
.else 
CFLAGS+= -Wall -DNO_GUI
.endif

.if !defined(NO_GUI)
GTK_LDFLAGS!=pkg-config gtk+-2.0 --libs
LDFLAGS+= ${GTK_LDFLAGS} -ldialog -lncurses
.else
LDFLAGS+= -ldialog -lncurses
.endif

.if defined(WITH_EFENCE)
LDFLAGS+= -L${LOCALBASE}/lib -lefence
.endif

PARSER_OBJ=	lex.yy.o parser.o
CFLAGS+= -DYY_NO_UNPUT

.if !defined (NO_GUI)
OBJECTS= ${PARSER_OBJ} main.o gtk_ui.o
.else 
OBJECTS= ${PARSER_OBJ} main.o
.endif

OBJECTS+= ncurses_ui.o

all:	$(OBJECTS)
	$(CC) $(LDFLAGS) -o thefish $(OBJECTS)

clean:
	rm -f *.o lex.yy.* thefish thefish.core

lex.yy.o:	lex.yy.c

lex.yy.c:	rc_conf.l
		lex rc_conf.l
