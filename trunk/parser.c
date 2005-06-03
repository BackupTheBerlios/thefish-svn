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
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include "parser.h"

static int parseline(RC_NODE *);
int yylex(void);

static int tmp_knobs, tmp_strings;

/* RC_NODEs that look like knobs but aren't */

static char *except[] = { "swapfile" , "pccard_ifconfig", "nisdomainname", 
			  "ip_portrange_first", "ip_portrange_last", "gif_interfaces",
			  "defaultrouter", "ipv6_defaultrouter", "keymap", "keyrate",
			  "keybell", "keychange", "cursor", "scrnmap", "font8x16",
			  "font8x14", "font8x8", "saver", "mousechar_start", "dumpdev",
			  "rand_irqs", "ipv6_default_interface", "ipv6_faith_prefix", 
			  "isdn_fsdev", "isdn_screenflags", "amd_map_program",
			  "gbde_devices", "extra_netfs_types", "pcvt_keymap", 
			  "pcvt_keydel", "pcvt_keyrate", "pcvt_lines",
			  "pcvt_blanktime", "pcvt_cursorh", "pcvt_cursorl", "sendmail_enable"
};


static int pending;

extern char *yytext;
extern FILE *yyin;

int
build_list(char *filename, RC_CONF *my_rc)
{
  RC_NODE scratch;
  RC_NODE *alpha;
  RC_NODE *current;
  RC_NODE *old;
  RC_NODE *foo, *bar;

  int retval;	

  tmp_knobs = 0;
  tmp_strings = 0;

  alpha = malloc(sizeof(RC_NODE));
  memset(alpha, 0, sizeof(RC_NODE));
  current = alpha;
  old = &scratch;

  pending = 0;
  retval = 0;

  yyin = fopen(filename, "r");

  /* First pass, build linked list of nodes */
  do {

    retval = parseline(current);
    old = current;
    current = malloc(sizeof(RC_NODE));
    memset(current, 0, sizeof(RC_NODE));
    old->next = current;
    current->next = NULL;

  } while (retval == 0);

  /* Second pass, build knob and strings list */
  if (tmp_knobs > 0) {

    my_rc->knobs_ptr = malloc(tmp_knobs * sizeof(RC_NODE));
    memset(my_rc->knobs_ptr, 0, (tmp_knobs * sizeof(RC_NODE)));

  } else {

    my_rc->knobs_ptr = malloc(sizeof(RC_NODE));
    memset(my_rc->knobs_ptr, 0, (sizeof(RC_NODE)));

  }

  if (tmp_strings > 0) {

    my_rc->string_ptr = malloc(tmp_strings * sizeof(RC_NODE));
    memset(my_rc->string_ptr, 0, (tmp_strings * sizeof(RC_NODE)));

  } else {

    my_rc->string_ptr = malloc(sizeof(RC_NODE));
    memset(my_rc->string_ptr, 0, (sizeof(RC_NODE)));

  }

  tmp_knobs = 0;
  tmp_strings = 0;

  /* Preserve original pointers */
  foo = my_rc->knobs_ptr;
  bar = my_rc->string_ptr;


  current = alpha;
  while (current->next != NULL) {

    if (isalnum((int)*current->name)) {

      if (current->knob == IS_KNOB) {

	*foo = *current;
	foo++;
	tmp_knobs++;

      } else {

	*bar = *current;
	bar++;
	tmp_strings++;

      }

    }

    current = current->next;

  }

  my_rc->knobs_size = tmp_knobs;
  my_rc->string_size = tmp_strings;
 
  fclose(yyin);
  return 0;

}

static int 
parseline(RC_NODE *current)
{
  char i;
  int delta = 0,compos = 0;
  int is_except = 0;
  int foo;
  int result = 1;

  if (pending == 0) {

    result = yylex();
    if (result == 0) return(-1);

  } else {

    pending = 0;

  }

  delta = 0;
  i = yytext[delta];

  while (i != '=') {

    current->name[delta] = i;
    delta++;
    i = yytext[delta];
  }

  current->name[delta] = (char) 0;

  /* Exception list */
  for (foo=0; foo < (sizeof(except)/sizeof(char *)); foo++) {

    if (!strncmp(current->name, except[foo], 255)) {

      is_except = 1;
      break;			

    }
  }


  result = yylex(); 
  if (result == 0) return(-1);
  
  /* Check for KNOB_YES or KNOB_NO */
  if (!strncmp(yytext, KNOB_YES, strlen(KNOB_YES))) {

    current->knob = IS_KNOB;
    current->knob_val = KNOB_IS_YES;
    current->knob_orig = KNOB_IS_YES;
    current->modified = MODIFIED_NO;
    tmp_knobs++;
	
  } else if (!strncmp(yytext, KNOB_NO, strlen(KNOB_NO)) && is_except == 0) {

    current->knob = IS_KNOB;
    current->knob_val = KNOB_IS_NO;
    current->knob_orig = KNOB_IS_NO;
    current->modified = MODIFIED_NO;
    tmp_knobs++;

  } else {

    current->knob = NOT_KNOB;
    delta = 0;
    i = yytext[delta];
    current->value[delta] = i;
    current->orig[delta] = i;
    delta++;

    i = yytext[delta];

    while (i != '"' && i != '\n' && i != (char) 0) {

      current->value[delta] = i;
      current->orig[delta] = i;
      delta++;
      i = yytext[delta];

    }

    current->value[delta] = (char) '"';
    current->orig[delta] = (char) '"';
    current->value[delta+1] = (char) 0;
    current->orig[delta+1] = (char) 0;
    current->modified = MODIFIED_NO;
    tmp_strings++;

  }


  result = yylex();
  if (result == 0) return(-1);
  i = *yytext;
  delta = 0;
  if (i == '#') {

    compos = 0;
    delta++;
    i = yytext[delta];
    while (i != '\n') {

      current->comment[compos] = i;
      delta++;
      compos++;
      i = yytext[delta];

    }

    current->comment[compos] = (char) 0;

  } else {

    pending = 1;

  }

  return(0);

}

/*
 * merge_lists receives two tuples of lists and merges the pairs, overriding the 
 * defaults list with the values in the rc list. It also allocates extra space
 * for possible user additions.
 */
int 
merge_lists(RC_CONF *my_rc_defaults, RC_CONF *my_rc)
{
  RC_NODE *rc_knobs_final;
  RC_NODE *rc_str_final;
  RC_NODE *baz, *bar, *aux;
  int total_nodes;
  int foo, j, k, asize;
  int node_present;

  total_nodes = my_rc_defaults->knobs_size;

  asize = (my_rc_defaults->knobs_size + my_rc->knobs_size) * 2;

#ifdef VERBOSE_CONSOLE
  printf("Allocated %i bytes for %i knobs.\n", asize * sizeof(RC_NODE), total_nodes);
#endif

  /* merge knobs */
  rc_knobs_final = malloc(asize * sizeof(RC_NODE));
  memset(rc_knobs_final, 0, (asize * sizeof(RC_NODE)));

  memcpy(rc_knobs_final, my_rc_defaults->knobs_ptr, 
	 my_rc_defaults->knobs_size * sizeof(RC_NODE));

  baz = my_rc->knobs_ptr;
  bar = rc_knobs_final;

  for (j = 0; j<total_nodes; j++) {

    baz = my_rc->knobs_ptr;
    for (foo = 0; foo < my_rc->knobs_size; foo++) {

      if (!strncmp(bar->name, baz->name, 255)) {
#ifdef VERBOSE_CONSOLE
	printf("Overridden---> %s\n", bar->name);
#endif
	bar->knob_val = baz->knob_val;
	bar->knob_orig = baz->knob_orig;
		
      }

      baz++;

    }

    bar++;

  }

  /* add values present in rc.conf but not in the defaults file */
  total_nodes = my_rc_defaults->knobs_size;
  baz = my_rc->knobs_ptr;

  for (j = 0; j < my_rc->knobs_size; j++) {

    node_present = 0;
    bar = my_rc_defaults->knobs_ptr;
   
    for (k = 0; k < my_rc_defaults->knobs_size; k++) {

      if (!strncmp(baz->name, bar->name, 255)) node_present = 1;
      bar++;

    }

    aux = rc_knobs_final;
    for (k = 0; k < total_nodes; k++) {

      if (!strncmp(baz->name, aux->name, 255)) {

#ifdef VERBOSE_CONSOLE
	printf("Dupe -> %s\n", aux->name);
#endif
	node_present = 1;
	aux->knob_val = baz->knob_val;
	aux->knob_orig = baz->knob_orig;

      }

      aux++;

    }

    if (node_present == 0) {

      memcpy((rc_knobs_final+total_nodes), baz, sizeof(RC_NODE));
      total_nodes++;

    }

    baz++;

  }

  my_rc_defaults->knobs_size = total_nodes;
  my_rc_defaults->knobs_ptr = rc_knobs_final;

  asize = (my_rc_defaults->string_size + my_rc->string_size) * 2;

#ifdef VERBOSE_CONSOLE
  printf("Allocated %i bytes for strings.\n", asize * sizeof(RC_NODE));
#endif

  /* merge strings */
  rc_str_final = malloc(asize * sizeof(RC_NODE));
  memset(rc_str_final, 0, (asize * sizeof(RC_NODE)));

  memcpy(rc_str_final, my_rc_defaults->string_ptr, 
	 my_rc_defaults->string_size * sizeof(RC_NODE));

  baz = my_rc->string_ptr;
  bar = rc_str_final;


  for (j = 0; j < my_rc_defaults->string_size; j++) {

    baz = my_rc->string_ptr;
    for (foo = 0; foo < my_rc->string_size; foo++) {

      if (!strncmp(bar->name, baz->name, 255)) {

#ifdef VERBOSE_CONSOLE
	fprintf(stderr,"Overridden---> %s\n", bar->name);
#endif
	memset(bar->value, 0, 255);
	strncpy(bar->value, baz->value, 255);

	memset(bar->orig, 0, 255);
	strncpy(bar->orig, baz->orig, 255);

      }

      baz++;

    }

    bar++;

  }

  /* add values present in rc.conf but not in the defaults file */
  total_nodes = my_rc_defaults->string_size;

  baz = my_rc->string_ptr;
  for (j = 0; j < my_rc->string_size; j++) {

    node_present = 0;
    bar = my_rc_defaults->string_ptr;

    for (k = 0; k < my_rc_defaults->string_size; k++) {

      if (!strncmp(baz->name, bar->name, 255)) node_present=1;
      bar++;

    }


    aux = rc_str_final;
    for (k=0; k < total_nodes; k++) {

      if (!strncmp(baz->name, aux->name, 255)) {

#ifdef VERBOSE_CONSOLE
	printf("Dupe -> %s\n", aux->name);
#endif

	node_present = 1;
	strncpy(aux->value, baz->value, 255);
	strncpy(aux->orig, baz->orig, 255);

      }

      aux++;

    }


    if (node_present == 0) {

#ifdef VERBOSE_CONSOLE
      printf("Adding new string: %s\n", baz->name);
#endif
      memcpy((rc_str_final+total_nodes), baz,sizeof(RC_NODE));
      total_nodes++;
    }

    baz++;

  }

  my_rc_defaults->string_size = total_nodes;
  my_rc_defaults->string_ptr = rc_str_final;

  /* everything went ok */
  return 0;

}

void
list_sort(RC_NODE *rc_data, int num)
{
  int i, j, retval, changed;
  RC_NODE *list;
  RC_NODE temp;

  list = rc_data;

  do {

    changed = 0;
    for (i = 0; i < num-1; i++) {
      for (j = i; j < num-1-i; j++) {

	retval = strncmp(list[j].name, list[j+1].name, 255);	

	if (retval >= 1) {
#ifdef VERBOSE_CONSOLE
	  printf("Sorting: %s (%x) > %s (%x)\n", list[j].name, list+j,
		 list[j+1].name, list+j+1);
#endif
	  memcpy(&temp, (list+j), sizeof(RC_NODE));
	  memcpy((list+j), (list+j+1), sizeof(RC_NODE));
	  memcpy((list+j+1), &temp, sizeof(RC_NODE));
	  changed = 1;
	}		
      }
    }

  } while (changed == 1);

}
