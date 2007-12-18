# $Id$
# This is a BSD Makefile, not a GNUMakefile!
#
X11BASE?=/usr/X11R6
LOCALBASE?=/usr/local
PTHREAD_LIBS?=-pthread

.if defined(WITH_GTK)
GTK_CFLAGS!=pkg-config gtk+-2.0 --cflags
CFLAGS+= -Wall -g -DWITH_GTK $(GTK_CFLAGS)
.endif

.if defined(NO_GUI)
CFLAGS+= -Wall -DNO_GUI
.endif

.if defined(WITH_QT)
CFLAGS+= -Wall -g -DWITH_QT -I$(X11BASE)/include
CXXFLAGS+= -Wall -g -DWITH_QT -I$(X11BASE)/include
.endif

LDFLAGS= -ldialog -lncurses

.if defined(WITH_GTK)
GTK_LDFLAGS!=pkg-config gtk+-2.0 --libs
LDFLAGS+= $(GTK_LDFLAGS) -ldialog -lncurses
.endif

.if defined(WITH_QT)
LDFLAGS+= -L$(X11BASE)/lib -lqt-mt $(PTHREAD_LIBS)
.endif

.if defined(WITH_EFENCE)
LDFLAGS+= -L$(LOCALBASE)/lib -lefence
.endif

PARSER_OBJ=	lex.yy.o parser.o
CFLAGS+= -DYY_NO_UNPUT

OBJECTS= $(PARSER_OBJ) main.o file.o ncurses_ui.o

.if defined(WITH_GTK)
OBJECTS+= gtk_ui.o
.endif

.if defined(WITH_QT)
OBJECTS+= qt_ui.o
.endif

all:	$(OBJECTS)
	$(CC) $(LDFLAGS) -o thefish $(OBJECTS)

about:	about.o
	$(CC) $(LDFLAGS) -o about about.o

clean:
	rm -f *.o lex.yy.* qt_ui_priv.moc thefish thefish.core

lex.yy.o:	lex.yy.c

lex.yy.c:	rc_conf.l
		lex rc_conf.l

qt_ui.o:	qt_ui_priv.moc qt_ui.C

qt_ui_priv.moc:	qt_ui_priv.h
		moc -o qt_ui_priv.moc qt_ui_priv.h

