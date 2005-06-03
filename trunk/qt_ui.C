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

#include <qcolor.h> 
#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qobject.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>

#include <fcntl.h>
#include <stdio.h>

#include "parser.h"
#include "thefish.h"

#include "qt_ui_priv.moc"

#include "fish64.xpm"

#define NOT_MODIFIED 0
#define IS_MODIFIED 1

int oldsize[2];
int dirty;

RC_NODE *my_rc_knobs, *my_rc_strings;
int my_num_knobs, my_num_strings;

QApplication *thefish;
QPushButton *SaveButton;
QMainWindow *mw;

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

  // It's more convenient to have these
  // as global.
  my_rc_knobs = rc_knobs;
  my_rc_strings = rc_strings;

  my_num_knobs = num_knobs;
  my_num_strings = num_strings;

  QMainWindow *mw = new QMainWindow;

  QVBox *vbox = new QVBox( mw, 0, 0);

  QTabWidget main_tab( vbox, 0, 0);

  QListView *knobs_table = new QListView;
  QListView *strings_table = new QListView;

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

  SaveButton= new QPushButton("&Save", hbuttons, 0);
  QPushButton AddButton("&Add", hbuttons, 0);
  QPushButton AboutButton("A&bout", hbuttons, 0);
  QPushButton QuitButton("&Quit",  hbuttons, 0);

  // No save for now...
  SaveButton->setEnabled(FALSE);

  QObject::connect( &QuitButton, SIGNAL(clicked()), &my_dialogs, SLOT(CheckSaved()));
  QObject::connect( &AboutButton, SIGNAL(clicked()), &my_dialogs, SLOT(ShowAbout()));
  QObject::connect( SaveButton, SIGNAL(clicked()), &my_dialogs, SLOT(DoSave()));

  //  QObject::connect( thefish, SIGNAL(lastWindowClosed()), &my_dialogs, SLOT(CheckSaved()));

  // We're now using human readable data, handle the migration
  // transparently for the user.
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
      // Set some default values
      oldsize[0]=400;
      oldsize[1]=480;

    }

  }

  // Build the table

  knobs_table->setColumnWidthMode ( 1, QListView::Maximum);
  knobs_table->setRootIsDecorated( FALSE );

  QColor mygray(240,240,240);

  for(i=num_knobs-1;i>=0;i--) {

    // No user comments yet
    (rc_knobs+i)->user_comment=0;

    foo = new QCheckListItem( knobs_table, (rc_knobs+i)->name, QCheckListItem::CheckBox);

    //    if(i%2==0) foo->setPaletteBackgroundColor(&mygray);



  if( (rc_knobs+i)->knob_val==KNOB_IS_NO) {

      foo->setOn(FALSE);

    } else {

      foo->setOn(TRUE);

    }

  }

  for(i=num_strings-1;i>=0;i--) {

    // No user comments yet
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
  int i;

  value = item->text(0);

#ifdef VERBOSE_CONSOLE
  fprintf(stderr, "You've set %s to %s\n",value.ascii(),text.ascii());
#endif

  for(i=0; i<=my_num_strings; i++) {

    if(strncmp((my_rc_strings+i)->name, value.ascii(), 255)==0) {

      strncpy((my_rc_strings+i)->value, text.ascii(), 255);

      if(strncmp((my_rc_strings+i)->orig, text.ascii(), 255)==0) {

	if(dirty>0) dirty--;
	(my_rc_strings+i)->modified=NOT_MODIFIED;

      } else {

	dirty++;
	(my_rc_strings+i)->modified=IS_MODIFIED;

      }

      if(dirty==0) {

	SaveButton->setEnabled(FALSE);

      } else {

	SaveButton->setEnabled(TRUE);

      }

      break;
    }
  }

}

void
TableCallbacks::KnobChanged(QListViewItem *item)
{
  QString value;
  QCheckListItem *foo;
  int i;

  value = item->text(0);

  foo = (QCheckListItem *) item;

  for(i=0; i<my_num_knobs; i++) {

    if(strncmp(value.ascii(), (my_rc_knobs+i)->name, 255)==0) {

      break;

    }

  }

  if(foo->isOn()) {

#ifdef VERBOSE_CONSOLE
    printf("Now %s is activated\n", value.ascii());
#endif

    (my_rc_knobs+i)->knob_val=KNOB_IS_YES;

    if((my_rc_knobs+i)->knob_orig==KNOB_IS_YES) {

      (my_rc_knobs+i)->modified=MODIFIED_NO;

      if(dirty>0) dirty--;

    } else {

      (my_rc_knobs+i)->modified=MODIFIED_YES;
      dirty++;

    }

  } else {

#ifdef VERBOSE_CONSOLE
    printf("Now %s is deactivated\n", value.ascii());
#endif

    (my_rc_knobs+i)->knob_val=KNOB_IS_NO;

    if((my_rc_knobs+i)->knob_orig==KNOB_IS_NO) {

      (my_rc_knobs+i)->modified=MODIFIED_NO;

      if(dirty>0) dirty--;

    } else {

      (my_rc_knobs+i)->modified=MODIFIED_YES;
      dirty++;

    }

  }


  if(dirty==0) {

    SaveButton->setEnabled(FALSE);

  } else {

    SaveButton->setEnabled(TRUE);

  }

}

void 
save_geometry(void)
{

  printf("Salvando el geometra\n");

}

// This function saves changes to $FISH_RC or 
// /etc/rc.conf
void
MyDialogs::DoSave()
{
  int i,not_committed;
  RC_NODE *work;
  FILE *fd;
  char *rc_file;
  char fish_header[255];
  char time_buf[255];
  char path_string[1024];
  time_t comm_time;

  not_committed=0;

  if(dirty>0) {

    work=my_rc_knobs;

    rc_file=getenv("FISH_RC");

    if(rc_file!=NULL) {

      fd=fopen(rc_file, "a");

    } else { 

      fd=fopen(RC_FILE, "a");

    }

    if(fd==NULL) {

      not_committed=1;			

    }

    if(not_committed==0) {

      comm_time=time(NULL);
      ctime_r(&comm_time,time_buf);
      snprintf(fish_header,255,"\n# The Fish generated deltas - %s",time_buf);
      fprintf(fd,fish_header);

      // modified knobs
      for(i=0;i<=my_num_knobs;i++) {

	if(work->modified==MODIFIED_YES) {

	  if(work->user_comment==0) {

	    fprintf(fd,"%s=%s\n", work->name,
		    work->knob_val == KNOB_IS_YES ? KNOB_YES : KNOB_NO);

	  } else {

	    fprintf(fd,"%s=%s\t# %s\n", work->name,
		    work->knob_val == KNOB_IS_YES ? KNOB_YES : KNOB_NO,
		    work->comment);

	  }

	  work->user_comment=0;
	  work->user_added=USER_ADDED_NO;
	  work->modified=MODIFIED_NO;
	  work->knob_orig=work->knob_val;

	}

	work++;

	} 

      // modified strings
      work=my_rc_strings;
      for(i=0;i<=my_num_strings;i++) {

	if(work->modified==MODIFIED_YES) {

	  if(work->user_comment==0) {

	    fprintf(fd,"%s=%s\n", work->name, work->value);

	  } else {

	    fprintf(fd,"%s=%s\t# %s\n", work->name, work->value,
		    work->comment);

	  }

	  work->user_comment=0;
	  work->modified=MODIFIED_NO;
	  work->user_added=USER_ADDED_NO;
	  strncpy(work->orig, work->value, 255);

	}

	work++;

      }  

      fclose(fd);

    }

      /* Pop up window */
    if(not_committed==1) {

      snprintf(path_string, 1023, "Can't open '%s' for writing. Changes not saved.",
	       rc_file!=NULL ? rc_file : RC_FILE);

      QMessageBox::critical( 0, "The Fish",
			     path_string);

    } else {

      QMessageBox::information( 0, "The Fish",
				"Changes saved.");
    }


  } else {

    QMessageBox::information( 0, "The Fish",
			      "There are no unsaved changes.");

  }

  if(not_committed==0) {

    dirty=0;
    SaveButton->setEnabled(FALSE);

  }

}
