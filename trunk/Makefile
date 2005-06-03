# $Id$
# This is a BSD Makefile, not a GNUMakefile!
#
LOCALBASE?=/usr/local
PTHREAD_LIBS?=-lc_r

.if !defined(NO_GUI) && !defined(WITH_QT)
GTK_CFLAGS!=pkg-config gtk+-2.0 --cflags
CFLAGS+= -Wall -g ${GTK_CFLAGS}
.endif

.if defined(NO_GUI)
CFLAGS+= -Wall -DNO_GUI
.endif

.if defined(WITH_QT)
CFLAGS+= -Wall -DWITH_QT -I/usr/X11R6/include
.endif

.if !defined(NO_GUI) && !defined(WITH_QT)
GTK_LDFLAGS!=pkg-config gtk+-2.0 --libs
LDFLAGS+= ${GTK_LDFLAGS} -ldialog -lncurses
.else
LDFLAGS+= -ldialog -lncurses
.endif

.if defined(WITH_QT)
LDFLAGS+= -L/usr/X11R6/lib -lqt-mt ${PTHREAD_LIBS}
.endif

.if defined(WITH_EFENCE)
LDFLAGS+= -L${LOCALBASE}/lib -lefence
.endif

PARSER_OBJ=	lex.yy.o parser.o
CFLAGS+= -DYY_NO_UNPUT

.if !defined(NO_GUI) && !defined(WITH_QT)
OBJECTS= ${PARSER_OBJ} main.o gtk_ui.o
.else 
OBJECTS= ${PARSER_OBJ} main.o
.endif

.if defined(WITH_QT)
OBJECTS+= qt_ui.o
.endif

OBJECTS+= ncurses_ui.o

all:	$(OBJECTS)
	$(CC) $(LDFLAGS) -o thefish $(OBJECTS)

clean:
	rm -f *.o lex.yy.* qt_ui_priv.moc thefish thefish.core

lex.yy.o:	lex.yy.c

lex.yy.c:	rc_conf.l
		lex rc_conf.l

qt_ui.o:	qt_ui_priv.moc qt_ui.C

qt_ui_priv.moc:	qt_ui_priv.h
		moc -o qt_ui_priv.moc qt_ui_priv.h

