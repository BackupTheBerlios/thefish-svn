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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>

#if !defined(NO_GUI)
#include <gtk/gtk.h>
#endif

#include "parser.h"
#include "thefish.h"

#if !defined(NO_GUI)
#include "gtk_ui.h"
#endif

#include "ncurses_ui.h"

void usage(void);
void purge(void);

char *defaults_rc_file,*rc_file;

int
main(int argc, char **argv)
{
  int counter,retval;
  int num_knobs, num_str;
  RC_NODE *rc_knobs;
  RC_NODE *rc_strings;
  RC_NODE *current;

  int num_knobs2, num_str2;
  RC_NODE *rc_knobs2;
  RC_NODE *rc_strings2;

#if !defined(NO_GUI)
  char *envtest;
#endif

  int ch, wantconsole;


  wantconsole=0;

#if !defined(NO_GUI)	
  envtest=getenv("DISPLAY");
  if(envtest==NULL) wantconsole=1;
#else
  wantconsole=1;
#endif

  while((ch = getopt(argc, argv, "cvh")) != -1) {

    switch(ch) {

    case 'c':
      wantconsole = 1;
      break;

    case 'v':
      printf("The Fish %s\n"
	     "Copyright (c) 2002-2004, Miguel Mendez."
	     " All rights reserved.\n",THE_FISH_VERSION);
      printf("Portions Copyright (c) 1995, Jordan Hubbard.\n");
      exit(EXIT_SUCCESS);

    case 'h':
    default:
      usage();

    }

    argc -= optind;
    argv += optind;

  }

#if !defined(NO_GUI)	
  if(wantconsole==0) gtk_init (&argc, &argv);
#endif

  defaults_rc_file=getenv("FISH_RC_DEFAULTS");
  rc_file=getenv("FISH_RC");
  rc_knobs=NULL;
  rc_strings=NULL;
  num_knobs=0;
  num_str=0;

  retval=build_list(defaults_rc_file!=NULL ? \
		    defaults_rc_file : RC_DEFAULTS_FILE,
		    0,&rc_knobs,&num_knobs,&rc_strings,&num_str);


  rc_knobs2=NULL;
  rc_strings2=NULL;
  num_knobs2=0;
  num_str2=0;


  retval=build_list(rc_file!=NULL ? \
		    rc_file : RC_FILE,
		    0,&rc_knobs2,&num_knobs2,&rc_strings2,&num_str2);


  purge();

  /* merge data */
  retval=merge_lists(&rc_knobs,&num_knobs,&rc_strings,&num_str,&rc_knobs2,
		     &num_knobs2,&rc_strings2,&num_str2);

  if(retval==-1) {

    perror("merge_lists()");
    exit(EXIT_FAILURE);

  }

  /* Sort the lists */
  list_sort(&rc_knobs,num_knobs);
  list_sort(&rc_strings,num_str);

  counter=0;
  current=NULL;

  /* Fix the problem with ncurses_ui not knowing about user_comments state */
  current=rc_knobs;
  for(counter=0;counter<num_knobs;counter++) {

    current->user_comment=0;
    current++;

  }

  current=rc_strings;
  for(counter=0;counter<num_str;counter++) {

    current->user_comment=0;
    current++;

  }

  /* Launch UI */
  if(wantconsole==0) {

#if !defined(NO_GUI)	
    create_gtk_ui(rc_knobs, num_knobs, rc_strings, num_str);
#endif

  } else {
	
    create_ncurses_ui(rc_knobs, num_knobs, rc_strings, num_str);

  }

  return 0;

}

void
usage(void)
{
  printf("usage: thefish [-c] [-v] [-h]\n");
  printf("\t-c\tforce console mode\n");
  printf("\t-v\tshow version and exit\n");
  printf("\t-h\tshow this screen\n");
  exit(EXIT_SUCCESS);
}
