/*
  Copyright (c) 2002-2004, Miguel Mendez. All rights reserved.

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

#include <dialog.h>

#define DMENU_NORMAL_TYPE	0x1
#define DMENU_RADIO_TYPE	0x2 
#define DMENU_CHECKLIST_TYPE	0x4
#define DMENU_SELECTION_RETURNS 0x8
#define MAX_MENU                15

typedef int Boolean;

typedef struct _dmenu {
  int type;                           /* What sort of menu we are     */
  char *title;                        /* Our title                    */
  char *prompt;                       /* Our prompt                   */
  char *helpline;                     /* Line of help at bottom       */
  char *helpfile;                     /* Help file for "F1"           */
  /* Array of menu items          */
#if __GNUC__>=3
  dialogMenuItem items[];
#else
  dialogMenuItem items[0];
#endif

} DMenu;

Boolean  dmenuOpen(DMenu *, int *, int *, int *, int *, Boolean );
Boolean  dmenuOpenSimple(DMenu *, Boolean );
Boolean  dmenuOpenSimple(DMenu *, Boolean );

void restorescr(WINDOW *);
WINDOW * savescr(void);
int dmenuSubmenu(dialogMenuItem *);
int dmenuExit(dialogMenuItem *);
int create_ncurses_ui(RC_NODE *, int, RC_NODE *, int);
int msgNoYes(char *fmt, ...);

