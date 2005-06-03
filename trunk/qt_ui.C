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

*/

#include <qobject.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qpushbutton.h>

#include "parser.h"
#include "thefish.h"

#include "qt_ui_priv.moc"

// C compat glue
extern "C" int create_qt_ui(RC_NODE *, int, RC_NODE *, int, int, char **);

extern "C" int
create_qt_ui(RC_NODE *rc_knobs,int num_knobs,RC_NODE *rc_strings,int num_str, int argc, char **argv)
{

  MiscDialogs mydialogs;

  QApplication thefish( argc, argv);

  QPushButton hello( "Hello world!", 0 );
  hello.resize( 100, 30 );

  QObject::connect( &hello, SIGNAL(clicked()), &mydialogs, SLOT(CheckSaved()) );

  thefish.setMainWidget( &hello );
  hello.show();
  return thefish.exec();


}

void 
MiscDialogs::CheckSaved()
{

    switch( QMessageBox::warning( 0, "The Fish",
				  "There are unsaved changes. "
				  "Quit anyway?\n\n",
				  "Yes",
				  "No", 0, 0, 1 ) ) 

      {

      case 0: // The user clicked the Yes button or pressed Enter
        // try again
        break;
      case 1: // The user clicked the No or pressed Escape
        // exit
        break;

      }

}
