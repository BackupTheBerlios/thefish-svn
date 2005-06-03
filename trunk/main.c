/*
  Copyright (c) 2002-2005, Miguel Mendez. All rights reserved.

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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sysexits.h>

#if defined(WITH_GTK)
#include <gtk/gtk.h>
#endif

#include "parser.h"
#include "thefish.h"

#if defined(WITH_GTK)
#include "gtk_ui.h"
#endif

#if defined(WITH_QT)
#include "qt_ui.h"
#endif

#include "ncurses_ui.h"

static void usage(void);
static void about(void);
void purge(void);

static char *defaults_rc_file, *rc_file;

int
main(int argc, char **argv)
{
  int counter,retval;
  RC_CONF *my_rc_defaults;
  RC_NODE *current;

  RC_CONF *my_rc;

#if !defined(NO_GUI)
  char *envtest;
#endif

  int ch, wantconsole;


  wantconsole = 0;

#if !defined(NO_GUI)
  envtest = getenv("DISPLAY");
  if (envtest == NULL) wantconsole=1;
#else
  wantconsole = 1;
#endif

  while ((ch = getopt(argc, argv, "cvh")) != -1) {

    switch (ch) {

    case 'c':
      wantconsole = 1;
      break;

    case 'v':
      about();
      /* NOTREACHED */
    case 'h':
    default:
      usage();
      /* NOTREACHED */
    }

  }

  argc -= optind;
  argv += optind;

#if defined(WITH_GTK)	
  if (wantconsole == 0) gtk_init (&argc, &argv);
#endif

  defaults_rc_file = getenv("FISH_RC_DEFAULTS");
  rc_file = getenv("FISH_RC");

  my_rc = malloc(sizeof(RC_CONF));
  my_rc_defaults = malloc(sizeof(RC_CONF));

  my_rc_defaults->knobs_ptr = NULL;
  my_rc_defaults->string_ptr = NULL;
  my_rc_defaults->knobs_size = 0;
  my_rc_defaults->string_size = 0;

  retval = build_list(defaults_rc_file != NULL ? \
		      defaults_rc_file : RC_DEFAULTS_FILE,
		      my_rc_defaults);

  my_rc->knobs_ptr = NULL;
  my_rc->string_ptr = NULL;
  my_rc->knobs_size = 0;
  my_rc->string_size = 0;

  retval = build_list(rc_file != NULL ? \
		      rc_file : RC_FILE,
		      my_rc);


  purge();

  /* merge data */
  retval = merge_lists(my_rc_defaults,
		       my_rc);

  if (retval == -1) {

    perror("merge_lists()");
    exit(EXIT_FAILURE);

  }

  /* Sort the lists */
  list_sort(my_rc_defaults->knobs_ptr, my_rc_defaults->knobs_size);
  list_sort(my_rc_defaults->string_ptr, my_rc_defaults->string_size);

  counter = 0;
  current = NULL;

  /* Fix the problem with ncurses_ui not knowing about user_comments state */
  current = my_rc_defaults->knobs_ptr;
  for (counter = 0; counter < my_rc_defaults->knobs_size; counter++) {

    current->user_comment = 0;
    current++;

  }

  current = my_rc_defaults->string_ptr;
  for (counter = 0; counter < my_rc_defaults->string_size; counter++) {

    current->user_comment = 0;
    current++;

  }

  /* Launch UI */
  if (wantconsole == 0) {

#if defined(WITH_GTK)
    create_gtk_ui(my_rc_defaults);
#endif

#if defined(WITH_QT)
    create_qt_ui(my_rc_defaults->knobs_ptr, my_rc_defaults->knobs_size,
		 my_rc_defaults->string_ptr, my_rc_defaults->string_size,
		 argc, argv);
#endif

  } else {
	
    create_ncurses_ui(my_rc_defaults->knobs_ptr, my_rc_defaults->knobs_size,
		      my_rc_defaults->string_ptr, my_rc_defaults->string_size);

  }

  return 0;

}

static void
usage(void)
{

  printf("usage: thefish [-c] [-v] [-h]\n");
  printf("\t-c\tforce console mode\n");
  printf("\t-v\tshow version and exit\n");
  printf("\t-h\tshow this screen\n");
  exit(EX_USAGE);

}

static void
about(void)
{

  printf("The Fish %s\n"
	 "Copyright (c) 2002-2005, Miguel Mendez."
	 " All rights reserved.\n", THE_FISH_VERSION);
  printf("Portions Copyright (c) 1995, Jordan Hubbard.\n");
  exit(EX_OK);

}
