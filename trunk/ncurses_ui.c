/*
  Copyright (c) 2002-2005, Miguel Mendez. All rights reserved.
  Copyright (c) 1995, Jordan Hubbard.  All rights reserved.

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

  This code includes some stuff from sysinstall's menu system, as I didn't feel 
  like reinventing the wheel. The beauty of the BSD license.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <time.h>
extern char *tzname[2];

#include <curses.h>
#include <dialog.h>

#include "parser.h"
#include "ncurses_ui.h"
#include "thefish.h"
#include "file.h"

/* Some defines here */
#define IS_DIRTY 1
#define NOT_DIRTY 0

/* Private protos and definitions */
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

static Boolean dmenuOpen(DMenu *, int *, int *, int *, int *, Boolean );
static Boolean dmenuOpenSimple(DMenu *, Boolean );
static Boolean dmenuOpenSimple(DMenu *, Boolean );

static void restorescr(WINDOW *);
static WINDOW * savescr(void);
static int dmenuSubmenu(dialogMenuItem *);
static int dmenuExit(dialogMenuItem *);
static int msgNoYes(char *fmt, ...);

static int dmenuSetVariable(dialogMenuItem *);
static int dmenuVarCheck(dialogMenuItem *);
static int commit_data(dialogMenuItem *);
static int add_entry(dialogMenuItem *);
static int show_about(dialogMenuItem *);
static int edit_callback(dialogMenuItem *);

static void ncurses_ui_main(void);

static Boolean exited;

static DMenu *MenuKnobs;
static DMenu *MenuStrings;

static int StatusLine;

static DMenu MenuInitial = {

  DMENU_NORMAL_TYPE,
  "The Fish Main Menu",
  "Select one of the options below by using the arrow keys or typing the\n"
  "first character of the option name you're interested in.  Invoke an\n"
  "option with [SPACE] or [ENTER].  To exit, use [TAB] to move to Exit.", 
  NULL,	/* help line */
  NULL,	/* help file */
  {	{ "Select" },
	{ "X Exit TheFish",NULL, NULL, dmenuExit },
	{ "Knobs","Enter the Knobs editor",NULL, dmenuSubmenu, NULL, NULL /* MenuKnobs */},
	{ "Strings","Enter the Strings editor",NULL, dmenuSubmenu, NULL,NULL /* MenuStrings */},
	{ "Add","Add a new entry",NULL, add_entry}, 
	{ "Save","Save changes",NULL, commit_data },
	{ "About","About The Fish",NULL, show_about},
	{ NULL } },

};

static char KnobsTitle[] = "Knobs Menu";
static char KnobsPrompt[] = "A checked entry means that knob is set to YES";
static char StringsTitle[] = "Strings Menu";
static char StringsPrompt[] = "Select a String you wish to edit";
static char back_str[] = "Back to Main";
static char change_str[] = "Change";
static char edit_str[] = "Edit Entry";

static RC_CONF rc_local;
static int gb_dirty = 0;

/* This one needs to be global, as it will be altered when adding new entries */
static dialogMenuItem *knobs_it, *str_it;

/*
 * Small stub to call the real code.
 */
int
create_ncurses_ui(RC_CONF *my_rc)
{

  rc_local.knobs_size = my_rc->knobs_size;
  rc_local.string_size = my_rc->string_size;
  rc_local.knobs_ptr = my_rc->knobs_ptr;
  rc_local.string_ptr = my_rc->string_ptr;

  ncurses_ui_main();

  return(0);

}

/*
 * This is the main function. It builds the menus and sets the appropriate
 * callbacks. Note that when we add entry we re-enter this function to generate
 * the new menus, as I found no other way of doing it with libdialog.
 */
static 
void ncurses_ui_main(void)
{
  WINDOW *scratch;
  int i;
  int screen_x, screen_y;
  int choice, scroll, curr, max, status;
  int really_exit;
  char *term_type;

  /* Get terminal and set title if it's an [axw]term */
  term_type = getenv("TERM");

  if (term_type != NULL && strncmp(++term_type, "term", 4) == 0) {

    printf("\033]0;The Fish "
	   THE_FISH_VERSION
	   "\007");

  }

  /* Get terminal geometry */
  scratch = initscr();
  screen_x = COLS;
  screen_y = LINES;
  endwin();

  /* Init the dialog system */
  init_dialog();

  /* Allocate memory for knobs + possible user additions */
  knobs_it = malloc(rc_local.knobs_size * 2 * sizeof(dialogMenuItem));
  memset(knobs_it, 0, rc_local.knobs_size * 2 * sizeof(dialogMenuItem));

  knobs_it[0].prompt = change_str;
  knobs_it[0].fire = NULL;
  knobs_it[1].prompt = back_str;

  /*
   * Generate knobs menu
   * it begins with 2 because the first two items are part of the
   * menu structure 
   */
  for (i = 2; i< rc_local.knobs_size + 2; i++) {

    knobs_it[i].title = "";
    knobs_it[i].prompt = rc_local.knobs_ptr[i - 2].name;
    knobs_it[i].checked = dmenuVarCheck; 
    knobs_it[i].fire = dmenuSetVariable;
    knobs_it[i].selected = NULL;
    knobs_it[i].data = malloc(16);
    knobs_it[i].aux = i - 2; /* keep track of knobs <-> menu entry relationship */

    if (rc_local.knobs_ptr[i - 2].knob_val == KNOB_IS_YES) {

      strcpy(knobs_it[i].data, "YES");

    } else {

      strcpy(knobs_it[i].data, "NO");

    }

    knobs_it[i].lbra = '[';
    knobs_it[i].mark = 'X';
    knobs_it[i].rbra = ']';

  }

  MenuKnobs = malloc(sizeof(DMenu) + (rc_local.knobs_size * 2 * sizeof(dialogMenuItem)));
  memset(MenuKnobs, 0, sizeof(DMenu) + (rc_local.knobs_size * 2 * sizeof(dialogMenuItem)));

  MenuKnobs->type = DMENU_CHECKLIST_TYPE;
  MenuKnobs->title = KnobsTitle;
  MenuKnobs->prompt = KnobsPrompt;
  MenuKnobs->helpline = NULL;
  MenuKnobs->helpfile = NULL;

  for (i = 0; i < rc_local.knobs_size + 2; i++) {

    memcpy(&MenuKnobs->items[i], &knobs_it[i], sizeof(dialogMenuItem));

  }

  /* Allocate memory for strings + possible user additions */
  str_it = malloc(rc_local.string_size * 2 * sizeof(dialogMenuItem));
  memset(str_it, 0, rc_local.string_size * 2 * sizeof(dialogMenuItem));

  str_it[0].prompt = edit_str;
  str_it[0].fire = NULL;
  str_it[1].prompt = back_str;
  str_it[1].fire = NULL;

  /* Generate strings menu */
  for (i = 2; i< rc_local.string_size + 2; i++) {

    str_it[i].prompt = rc_local.string_ptr[i - 2].name;
    str_it[i].title = rc_local.string_ptr[i - 2].value;
    str_it[i].checked = NULL;
    str_it[i].fire = edit_callback;
    str_it[i].selected = NULL;
    str_it[i].data = NULL;
    str_it[i].lbra = '[';
    str_it[i].mark = 'X';
    str_it[i].rbra = ']';
    str_it[i].aux = i - 2; /* keep track of str <-> menu entry relationship */

  }


  MenuStrings = malloc(sizeof(DMenu) + (rc_local.string_size * 2 * sizeof(dialogMenuItem)));
  memset(MenuStrings, 0, sizeof(DMenu) + (rc_local.string_size * 2 * sizeof(dialogMenuItem)));

  MenuStrings->type = DMENU_NORMAL_TYPE;
  MenuStrings->title = StringsTitle;
  MenuStrings->prompt = StringsPrompt;
  MenuStrings->helpline = NULL;
  MenuStrings->helpfile = NULL;

  for (i = 0; i < rc_local.string_size + 2; i++) {

    memcpy(&MenuStrings->items[i], &str_it[i], sizeof(dialogMenuItem));

  }

  MenuInitial.items[2].data = MenuKnobs;
  MenuInitial.items[3].data = MenuStrings;

  dialog_clear();
  status = 0;
  really_exit = 0;

  while (really_exit == 0) {

    choice = scroll = curr = max = 0;
    dmenuOpen(&MenuInitial, &choice, &scroll, &curr, &max, TRUE);

    if (gb_dirty == 1) {

      if (!msgNoYes("There are unsaved changes. Exit anyway?\n")) really_exit = 1;

    } else {

      really_exit = 1;

    }

  }

  end_dialog();

}

/* Jordan's code */
static int
dmenuSubmenu(dialogMenuItem *tmp)
{

  return (dmenuOpenSimple((DMenu *)(tmp->data), TRUE) ? DITEM_SUCCESS : DITEM_FAILURE);

}

/* Jordan's */
static int
dmenuExit(dialogMenuItem *tmp)
{

  exited = TRUE;
  return DITEM_LEAVE_MENU;

}

/* Also by Jordan, taken from sysinstall */
static Boolean
dmenuOpenSimple(DMenu *menu, Boolean buttons)
{

  int choice, scroll, curr, max;

  choice = scroll = curr = max = 0;
  return dmenuOpen(menu, &choice, &scroll, &curr, &max, buttons);

}

/* Jordan's */
static WINDOW *
savescr(void)
{

  WINDOW *w;

  w = dupwin(newscr);
  return w;

}

/* Jordan's */
static void
restorescr(WINDOW *w)
{

  touchwin(w);
  wrefresh(w);
  delwin(w);

}

/* Jordan's */
static int
menu_height(DMenu *menu, int n)
{

  int max;
  char *t;

  max = MAX_MENU;
  if (StatusLine > 24)
    max += StatusLine - 24;

  for (t = menu->prompt; *t; t++) {
  
    if (*t == '\n')
      --max;

  }

  return n > max ? max : n;

}

/* Jordan's */
static Boolean
dmenuOpen(DMenu *menu, int *choice, int *scroll, int *curr, int *max, Boolean buttons)
{

  int n, rval = 0;
  dialogMenuItem *items;

  items = menu->items;
  if (buttons)
    items += 2;
  /* Count up all the items */
  for (n = 0; items[n].title; n++);

  while (1) {

    WINDOW *w = savescr();

    /* Any helpful hints, put 'em up! */
    use_helpline(menu->helpline);

    /* use_helpfile(systemHelpFile(menu->helpfile, buf)); */
    dialog_clear_norefresh();

    /* Pop up that dialog! */
    if (menu->type & DMENU_NORMAL_TYPE)

      rval = dialog_menu((unsigned char *)menu->title, (unsigned char *)menu->prompt, -1, -1,
			 menu_height(menu, n), -n, items, (char *)buttons, choice, scroll);

    else if (menu->type & DMENU_RADIO_TYPE)

      rval = dialog_radiolist((unsigned char *)menu->title, (unsigned char *)menu->prompt, -1, -1,
			      menu_height(menu, n), -n, items, (char *)buttons);

    else if (menu->type & DMENU_CHECKLIST_TYPE)

      rval = dialog_checklist((unsigned char *)menu->title, (unsigned char *)menu->prompt, -1, -1,
			      menu_height(menu, n), -n, items, (char *)buttons);
    else

      fprintf(stderr,"Menu: `%s' is of an unknown type\n", menu->title);

    if (exited) {
      exited = FALSE;
      restorescr(w);
      return TRUE;

    }

    else if (rval) {
      restorescr(w);
      return FALSE;

    }

    else if (menu->type & DMENU_SELECTION_RETURNS) {
      restorescr(w);
      return TRUE;

    }

  }

}

/* Jordan's */
static int
msgNoYes(char *fmt, ...)
{
  va_list args;
  char *errstr;
  int ret;
  WINDOW *w = savescr();

  errstr = (char *)alloca(FILENAME_MAX);
  va_start(args, fmt);
  vsnprintf(errstr, FILENAME_MAX, fmt, args);
  va_end(args);
  use_helpline(NULL);
  use_helpfile(NULL);

  ret = dialog_noyes("User Confirmation Requested", errstr, -1, -1);
  restorescr(w);
  return ret;

}

/* Save changes to rc.conf or $FISH_RC */
static int
commit_data(dialogMenuItem *tmp)
{

  int i;
  char *rc_file;

  rc_file = getenv("FISH_RC");

  if (gb_dirty == 1) {

    i = save_changes(rc_local.knobs_ptr, rc_local.knobs_size, 
		     rc_local.string_ptr, rc_local.string_size);

    if (i == 0) {

      dialog_notify("Changes saved.");

    } else {

      dialog_notify("Could not save data!");

    }

  } else {

    dialog_notify("There are no unsaved changes");

  }

  gb_dirty = 0;

  return TRUE;

}

/* Usual About dialog */
static int
show_about(dialogMenuItem *tmp)
{

  dialog_notify("              The Fish "
		THE_FISH_VERSION
		"\n"
		"      A user friendly ncurses/GTK+/Qt rc.conf editor.\n"
		"Copyright (c) 2002-2005, Miguel Mendez. All rights reserved.\n"
		"        Portions Copyright (c) 1995, Jordan Hubbard.");
  return TRUE;

}

/*
 * This callback syncs menu info with RC_NODE structures
 * when the user modifies a knob
 */
static int
dmenuSetVariable(dialogMenuItem *tmp)
{

  if (!strcmp(tmp->data, "YES")) {

    strcpy(tmp->data, "NO");
    rc_local.knobs_ptr[tmp->aux].knob_val = KNOB_IS_NO;		

  } else {

    strcpy(tmp->data, "YES");
    rc_local.knobs_ptr[tmp->aux].knob_val = KNOB_IS_YES;	

  }

  if (rc_local.knobs_ptr[tmp->aux].knob_val == rc_local.knobs_ptr[tmp->aux].knob_orig
      && rc_local.knobs_ptr[tmp->aux].user_added == USER_ADDED_NO) {

    rc_local.knobs_ptr[tmp->aux].modified = MODIFIED_NO;

  } else {

    rc_local.knobs_ptr[tmp->aux].modified = MODIFIED_YES;

  }

  gb_dirty = 1;
  return DITEM_SUCCESS;

}

static int
dmenuVarCheck(dialogMenuItem *item)
{

  if (!strcmp(item->data, "YES")) {

    return TRUE;

  } else {

    return FALSE;

  }

}

/*
 * The edit callback, called when the user wants to
 * change a string.
 */
static int
edit_callback(dialogMenuItem *item)
{

  int retcode,i;
  char result[256];

  i = item->aux;
  strncpy(result, rc_local.string_ptr[i].value, 255);
  retcode = dialog_inputbox(item->prompt, "", 8, 64, result);

  if (retcode == 0) {

    /* Sanity check */
    if (result[0] != '"' || result[strlen(result)-1] != '"') {

      dialog_notify("Value must begin and end with '\"'.");

    } else {

      strncpy(rc_local.string_ptr[i].value, result, 255);

      if (!strncmp(rc_local.string_ptr[i].value, rc_local.string_ptr[i].orig, 255)
	  && rc_local.string_ptr[i].user_added == USER_ADDED_NO) {

	rc_local.string_ptr[i].modified = MODIFIED_NO;

      } else {

	rc_local.string_ptr[i].modified = MODIFIED_YES;

      }

      gb_dirty = 1;

    }

  }

  return DITEM_SUCCESS;

}

static int
add_entry(dialogMenuItem *item)
{
  int i, dupe, quot;
  int retcode;
  char new_name[4096];
  char new_value[4096];
  char new_comment[4096];

  memset(new_name, 0, 4096);
  memset(new_value, 0, 4096);
  memset(new_comment, 0, 4096);
  sprintf(new_value, "\"\"");


  do {

    dupe = 0;
    retcode = dialog_inputbox("Name of the new entry", "", 8, 64, new_name);
    if (retcode != 0) return DITEM_SUCCESS;

    /* Check for duplicate entries */
    for (i = 0; i < rc_local.knobs_size; i++) {

      if (!strncmp(rc_local.knobs_ptr[i].name, new_name, 255)) {

	dupe = 1;
	break;

      }

    }

    for (i = 0; i < rc_local.string_size; i++) {

      if (!strncmp(rc_local.string_ptr[i].name, new_name, 255)) {
	dupe = 1;
	break;

      }

    }

    if (dupe == 1) {
	
      dialog_notify("Duplicated entry.");

    }

  } while (dupe == 1);


  do {

    quot = 0;
    retcode = dialog_inputbox("Value (Can be YES, NO or a string)",
			      "", 8, 64, new_value);

    /* User has aborted the add operation */
    if (retcode != 0) return DITEM_SUCCESS;

    if (new_value[0] != '"' || new_value[strlen(new_value)-1] != '"') {

      dialog_notify("Value must begin and end with '\"'.");
      quot = 1;

    }

  } while (quot == 1);

  retcode = dialog_inputbox("Optional comment (leave empty for none)",
			    "", 8, 64, new_comment);

  /* User has aborted the add operation */
  if (retcode != 0) return DITEM_SUCCESS;

  /* Check if new entry is a KNOB or not */
  if (!strncasecmp(new_value, KNOB_YES, 255) || !strncasecmp(new_value, KNOB_NO, 255)) {

    strncpy(rc_local.knobs_ptr[rc_local.knobs_size].name, new_name, 255);
    if (!strncasecmp(new_value, KNOB_YES, 255)) {

      rc_local.knobs_ptr[rc_local.knobs_size].knob_val = KNOB_IS_YES;
      rc_local.knobs_ptr[rc_local.knobs_size].knob_orig = KNOB_IS_YES;
      rc_local.knobs_ptr[rc_local.knobs_size].modified = MODIFIED_YES;
      rc_local.knobs_ptr[rc_local.knobs_size].user_added = USER_ADDED_YES;

      if (strlen(new_comment) > 0) {

	strncpy(rc_local.knobs_ptr[rc_local.knobs_size].comment, new_comment, 255);
	rc_local.knobs_ptr[rc_local.knobs_size].user_comment = 1;

      }

      gb_dirty = 1;

    } else {

      rc_local.knobs_ptr[rc_local.knobs_size].knob_val = KNOB_IS_NO;
      rc_local.knobs_ptr[rc_local.knobs_size].knob_orig = KNOB_IS_NO;
      rc_local.knobs_ptr[rc_local.knobs_size].modified = MODIFIED_YES;
      rc_local.knobs_ptr[rc_local.knobs_size].user_added = USER_ADDED_YES;
      gb_dirty = 1;

      if (strlen(new_comment) > 0) {

	strncpy(rc_local.knobs_ptr[rc_local.knobs_size].comment, new_comment, 255);
	rc_local.knobs_ptr[rc_local.knobs_size].user_comment = 1;

      }

    }

    rc_local.knobs_size++;

    /* Is a string */
  } else {

    strncpy(rc_local.string_ptr[rc_local.string_size].name, new_name, 255);
    strncpy(rc_local.string_ptr[rc_local.string_size].value, new_value, 255);
    strncpy(rc_local.string_ptr[rc_local.string_size].orig, new_value, 255);
    rc_local.string_ptr[rc_local.string_size].modified = MODIFIED_YES;
    rc_local.string_ptr[rc_local.string_size].user_added = USER_ADDED_YES;
    gb_dirty = 1;

    if (strlen(new_comment) > 0) {

      strncpy(rc_local.string_ptr[rc_local.string_size].comment, new_comment, 255);
      rc_local.string_ptr[rc_local.string_size].user_comment = 1;

    }

    rc_local.string_size++;			

  }

  /* HACK HACK HACK HACK */
  /* Since there are new entries in the menus, we need to regenerate them, ugly hack */
  end_dialog();

  free(knobs_it);
  free(MenuKnobs);
  free(str_it);
  free(MenuStrings);

  ncurses_ui_main();
  end_dialog();
  exit(0);

}
