/*
  Copyright (c) 2004, Miguel Mendez. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer. 
  * Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation 
  and/or other materials provided with the distribution. 

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  $Id$

  Note: This code is quite suboptimal for two reasons: 1) I'm still learning how QT 
  works and 2) I'm not really a fan of C++. To be honest I hate C++, but QT makes it
  almost bearable.

*/

#include <qhbox.h>
#include <qvbox.h>
#include <qlistview.h>
#include <qobject.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qpushbutton.h>
#include <qtabwidget.h>

#include <fcntl.h>
#include <stdio.h>

#include "parser.h"
#include "thefish.h"

#include "qt_ui_priv.moc"

/* Window geometry */
/* Deprecated for file, we
 * now use human readable data
 */
int oldsize[2];

int dirty;

QApplication *thefish;

// C compat glue
extern "C" int create_qt_ui(RC_NODE *, int, RC_NODE *, int, int, char **);

void save_geometry(void);

extern "C" int
create_qt_ui(RC_NODE *rc_knobs, int num_knobs,
	     RC_NODE *rc_strings, int num_str, 
	     int argc, char **argv)
{
  int i;
  char *homedir;
  char temp[FILENAME_MAX];
  int fd;
  FILE *fp;

  MiscDialogs mydialogs;

  QApplication *thefish = new QApplication( argc, argv);

  QMainWindow *mw = new QMainWindow;


  QVBox *vbox = new QVBox( mw, 0, 0);

  QTabWidget main_tab( vbox, 0, 0);

  QListView *knobs_table = new QListView;
  QListView *strings_table = new QListView;

  knobs_table->addColumn("S", -1);
  knobs_table->addColumn("Name", -1);
  knobs_table->addColumn("Enabled",-1);

  strings_table->addColumn("S", -1);
  strings_table->addColumn("Name", -1);
  strings_table->addColumn("Value",-1);

  main_tab.addTab(knobs_table, "Knobs");
  main_tab.addTab(strings_table, "Strings");

  QHBox *hbuttons = new QHBox( vbox, 0, 0);

  QPushButton SaveButton("&Save", hbuttons, 0);
  QPushButton AddButton("&Add", hbuttons, 0);
  QPushButton AboutButton("A&bout", hbuttons, 0);
  QPushButton QuitButton("&Quit",  hbuttons, 0);

  // No save for now...
  SaveButton.setEnabled(FALSE);

  QObject::connect( &QuitButton, SIGNAL(clicked()), &mydialogs, SLOT(CheckSaved()));
  QObject::connect( &AboutButton, SIGNAL(clicked()), &mydialogs, SLOT(ShowAbout()));

  /* Set the size.
   * We're now using human readable data, handle the migration
   * transparently for the user.
   */
  homedir=getenv("HOME");

  if(homedir!=NULL) {

    snprintf(temp, FILENAME_MAX, "%s/%s", homedir, ".thefishrc");
    fd=open(temp, O_RDONLY, 0);

    if(fd!=-1) {

      i=lseek(fd, 0, SEEK_END);
      lseek(fd, 0, SEEK_SET);

      if(i==sizeof(oldsize)) {

	read(fd, &oldsize[0], sizeof(oldsize));
	close(fd);

      } else {

	fp=fdopen(fd, "r");
	fscanf(fp, "geometry=%i,%i", &oldsize[0], &oldsize[1]);
	fclose(fp);

      }

    } else  {
      /* Set some default values */
      oldsize[0]=400;
      oldsize[1]=480;

    }

  }

  mw->setCaption( "The Fish " THE_FISH_VERSION);
  mw->setCentralWidget( vbox );
  thefish->setMainWidget(mw);
  mw->show();
  mw->resize(oldsize[0], oldsize[1]);

  dirty=0;

  return thefish->exec();


}

void 
MiscDialogs::CheckSaved()
{
  int retcode;

  if(dirty>0) {

    retcode = QMessageBox::warning( 0, "The Fish",
				    "There are unsaved changes. "
				    "Quit anyway?\n\n",
				    "Yes",
				    "No", 0, 0, 1 );

    if(retcode == 0) {
			
      save_geometry();
      thefish->exit(0);

    }

  } else {

    save_geometry();
    thefish->exit(0);

  }

}

void
MiscDialogs::ShowAbout()
{

  QMessageBox::about( 0, "About The Fish",
		      "The Fish "
		      THE_FISH_VERSION
		      "\nCopyright (c) 2002-2004, "
		      "Miguel Mendez\n"
		      "Shark icon (c) 2001-2003, Alan Smith\n"
		      "E-Mail: <flynn@energyhq.es.eu.org>\n"
		      "http://www.energyhq.es.eu.org/thefish.html\n"
		      ); 

}

void 
save_geometry(void)
{

  printf("Salvando el geometra\n");

}

