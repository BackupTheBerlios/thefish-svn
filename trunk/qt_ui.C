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
#include <qlabel.h>
#include <qlistview.h>
#include <qobject.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qpushbutton.h>
#include <qtable.h>
#include <qtabwidget.h>

#include <fcntl.h>
#include <stdio.h>

#include "parser.h"
#include "thefish.h"

#include "qt_ui_priv.moc"

#include "fish64.xpm"

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
	     RC_NODE *rc_strings, int num_strings, 
	     int argc, char **argv)
{
  int i;
  char *homedir;
  char temp[FILENAME_MAX];
  int fd;
  FILE *fp;
  QListViewItem *element;
  QCheckListItem *foo;

  MyDialogs my_dialogs;
  TableCallbacks my_tablecallbacks;

  QApplication *thefish = new QApplication( argc, argv);

  QMainWindow *mw = new QMainWindow;


  QVBox *vbox = new QVBox( mw, 0, 0);

  QTabWidget main_tab( vbox, 0, 0);

  QListView *knobs_table = new QListView;
  QListView *strings_table = new QListView;

  knobs_table->setSorting(-1);
  strings_table->setSorting(-1);

  strings_table->setAllColumnsShowFocus(TRUE);

  knobs_table->addColumn("Variable Name", -1);

  strings_table->addColumn("Name", -1);
  strings_table->addColumn("Value",-1);

  QObject::connect( strings_table, SIGNAL(itemRenamed(QListViewItem*, int, const QString &)),
		    &my_tablecallbacks, SLOT(StringChanged(QListViewItem*, int, const QString &)));


  QObject::connect( knobs_table, SIGNAL(clicked(QListViewItem *)),
		    &my_tablecallbacks, SLOT(KnobChanged(QListViewItem*)));


  main_tab.addTab(knobs_table, "Knobs");
  main_tab.addTab(strings_table, "Strings");

  QHBox *hbuttons = new QHBox( vbox, 0, 0);

  QPushButton SaveButton("&Save", hbuttons, 0);
  QPushButton AddButton("&Add", hbuttons, 0);
  QPushButton AboutButton("A&bout", hbuttons, 0);
  QPushButton QuitButton("&Quit",  hbuttons, 0);

  // No save for now...
  SaveButton.setEnabled(FALSE);

  QObject::connect( &QuitButton, SIGNAL(clicked()), &my_dialogs, SLOT(CheckSaved()));
  QObject::connect( &AboutButton, SIGNAL(clicked()), &my_dialogs, SLOT(ShowAbout()));

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

// Build the table

  knobs_table->setColumnWidthMode ( 1, QListView::Maximum);
  knobs_table->setRootIsDecorated( FALSE );


  for(i=num_knobs-1;i>=0;i--) {

    /* No user comments yet */
    (rc_knobs+i)->user_comment=0;

    foo = new QCheckListItem( knobs_table, (rc_knobs+i)->name, QCheckListItem::CheckBox);

    if( (rc_knobs+i)->knob_val==KNOB_IS_NO) {

      foo->setOn(FALSE);
      
    } else {

      foo->setOn(TRUE);

    }

  }


  for(i=num_strings-1;i>=0;i--) {

    /* No user comments yet */
    (rc_strings+i)->user_comment=0;

    element = new QListViewItem( strings_table, (rc_strings+i)->name, (rc_strings+i)->value);
    element->setRenameEnabled( 0, FALSE);
    element->setRenameEnabled( 1, TRUE);

  }

  // Set the app icon
  QPixmap my_icon((const char **) fish64_xpm);

  mw->setIcon((const QPixmap ) my_icon);

  mw->setCaption( "The Fish " THE_FISH_VERSION);
  mw->setCentralWidget( vbox );
  thefish->setMainWidget(mw);
  mw->show();
  mw->resize(oldsize[0], oldsize[1]);

  dirty=0;

  return thefish->exec();


}

void 
MyDialogs::CheckSaved()
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
MyDialogs::ShowAbout()
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
TableCallbacks::StringChanged(QListViewItem * item, int col, const QString & text)
{
  QString value;

  value = item->text(0);
  printf("Se ha cambiado %s a %s\n",value.ascii(),text.ascii());

}

void
TableCallbacks::KnobChanged(QListViewItem *item)
{
  QString value;
  QCheckListItem *foo;

  value = item->text(0);

  foo = (QCheckListItem *) item;

  if(foo->isOn()) {

  printf("Ahora %s esta activo\n", value.ascii() );

  }
}

void 
save_geometry(void)
{

  printf("Salvando el geometra\n");

}

