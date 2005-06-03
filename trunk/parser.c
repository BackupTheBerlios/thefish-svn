/*
Copyright (c) 2002, Miguel Mendez. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer. 
	* Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation 
	and/or other materials provided with the distribution. 
	* Neither the name of Miguel Mendez nor the names of his contributors
	may be used to endorse or promote products derived from this software 
	without specific prior written permission.

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

#include "parser.h"

int bufpos,tmp_knobs,tmp_strings;
int bufsize;

/* RC_NODEs that look like knobs but aren't */

char *except[]={ "swapfile" , "pccard_ifconfig", "nisdomainname", 
				 "ip_portrange_first", "ip_portrange_last", "gif_interfaces",
				 "defaultrouter", "ipv6_defaultrouter", "keymap", "keyrate",
				 "keybell", "keychange", "cursor", "scrnmap", "font8x16",
				 "font8x14", "font8x8", "saver", "mousechar_start", "dumpdev",
				 "rand_irqs", "ipv6_default_interface", "ipv6_faith_prefix", 
				 "isdn_fsdev", "isdn_screenflags", "amd_map_program",
				 "pcvt_keymap","pcvt_keydel","pcvt_keyrate","pcvt_lines",
				 "pcvt_blanktime","pcvt_cursorh","pcvt_cursorl","sendmail_enable"};


int
build_list(char *buffer,int len,RC_NODE **rc_knobs,int *num_knobs,RC_NODE **rc_str,int *num_str)
{
  RC_NODE scratch;
  RC_NODE *alpha;
  RC_NODE *current;
  RC_NODE *old;
  RC_NODE *foo, *bar;

  int retval;	

  bufsize=len;
  tmp_knobs=0;
  tmp_strings=0;

  alpha=malloc(sizeof(RC_NODE));

  bufpos=0;
  current=alpha;
  old=&scratch;

  /* First pass, build linked list of nodes */
  while(bufpos<len && bufpos!=-1) {

	retval=0;
	while(retval==0) {
	  retval=parseline(buffer,current);
	  if(retval==-1) break;
	}

	if(retval==-1) break;
	old=current;
	current=malloc(sizeof(RC_NODE));
	old->next=current;
	current->next=NULL;

  }

  /* Second pass, build knob and strings list */
  *rc_knobs=malloc(tmp_knobs*sizeof(RC_NODE));
  *rc_str=malloc(tmp_strings*sizeof(RC_NODE));
  *num_knobs=tmp_knobs;
  *num_str=tmp_strings;

  /* Preserve original pointers */
  foo=*rc_knobs;
  bar=*rc_str;

  current=alpha;
  while(current->next!=NULL) {

	if(current->knob==IS_KNOB) {

	  *foo=*current;
	  foo++;

	} else {

	  *bar=*current;
	  bar++;

	}

	current=current->next;

  }

  return 0;

}

int 
parseline(char *buffer,RC_NODE *current)
{
  char i;
  int delta=0,compos=0;
  int is_except=0;
  int foo;

  memset(current->comment,0,255);

  if( (!strncmp((buffer+bufpos),"if [",4)) || ((char) *(buffer+bufpos) == '\0') ) return -1;

  if( (char) *(buffer+bufpos) == '\n' ) {
	bufpos++;
	return 0;
  }

  if((char) *(buffer+bufpos) == '#' \
	 || (char) *(buffer+bufpos) == ' '\
	 || (char) *(buffer+bufpos) == '\t') {

	/* Skip comment */
	i=*(buffer+bufpos);
	
	while(i!='\n') {
	  bufpos++;
	  i=*(buffer+bufpos);
	}

	bufpos++;
	return(0);
  }

  delta=0;
  i=*(buffer+bufpos);
  while(i!='=') {

	current->name[delta]=i;
	delta++;
	bufpos++;
	i=*(buffer+bufpos);
  }

  current->name[delta]=(char) 0;

  /* Skip the = sign */
  bufpos++;

  /* Exception list */
  for(foo=0;foo<(sizeof(except)/sizeof(char *));foo++) {

	if(!strncmp(current->name,except[foo],255)) {
	  is_except=1;
	  break;			
		}
  }

  /* Check for KNOB_YES or KNOB_NO */
  if(!strncmp((buffer+bufpos),KNOB_YES,strlen(KNOB_YES))) {

	current->knob=IS_KNOB;
	current->knob_val=KNOB_IS_YES;
	current->knob_orig=KNOB_IS_YES;
	current->modified=MODIFIED_NO;
	bufpos+=strlen(KNOB_YES);
	tmp_knobs++;
	
  } else if(!strncmp((buffer+bufpos),KNOB_NO,strlen(KNOB_NO)) && is_except==0) {

	current->knob=IS_KNOB;
	current->knob_val=KNOB_IS_NO;
	current->knob_orig=KNOB_IS_NO;
	current->modified=MODIFIED_NO;
	bufpos+=strlen(KNOB_NO);
	tmp_knobs++;

  } else {

	current->knob=NOT_KNOB;
	delta=0;
	i=*(buffer+bufpos);
	current->value[delta]=i;
	current->orig[delta]=i;
	delta++;
	bufpos++;
	i=*(buffer+bufpos);

	while(i!='"' && i!='\n' && i!=(char) 0) {

	  current->value[delta]=i;
	  current->orig[delta]=i;
	  delta++;
	  bufpos++;
	  i=*(buffer+bufpos);

	}

	current->value[delta]=(char) '"';
	current->orig[delta]=(char) '"';
	current->value[delta+1]=(char) 0;
	current->orig[delta+1]=(char) 0;
	current->modified=MODIFIED_NO;
	tmp_strings++;
	bufpos++;

  }

  i=*(buffer+bufpos);

  while(i!='\n' && bufpos<bufsize) {

	bufpos++;
	i=*(buffer+bufpos);

	/* New code (06/15/02) , insert comment */
	if(i=='#') {

	  compos=0;
	  bufpos++;
	  i=*(buffer+bufpos);
	  while(i!='\n' && bufpos<bufsize) {

		current->comment[compos]=i;
		bufpos++;
		compos++;
		i=*(buffer+bufpos);

	  }

	  current->comment[compos]=(char) 0;
	}		

  }

  bufpos++;
	
  return(1);

}

/* merge_lists receives two tuples of lists and merges the pairs, overriding the 
 * defaults list with the values in the rc list
 */
int 
merge_lists(RC_NODE **rc_knobs,int *num_knobs,RC_NODE **rc_str,int *num_str,
			RC_NODE **rc_knobs2,int *num_knobs2,RC_NODE **rc_str2,int *num_str2)
{
  RC_NODE *rc_knobs_final;
  RC_NODE *rc_str_final;
  RC_NODE *baz, *bar;
  int total_nodes;
  int foo,j,k;
  int node_present;

  total_nodes=*num_knobs;

  /* merge knobs */
  rc_knobs_final=malloc( ((*num_knobs)+(*num_knobs2)) *sizeof(RC_NODE));

  memcpy(rc_knobs_final,(*rc_knobs),(*num_knobs)*sizeof(RC_NODE));

  baz=*rc_knobs2;
  bar=rc_knobs_final;


  for(j=0;j<(*num_knobs);j++) {

	baz=*rc_knobs2;
	for(foo=0;foo<(*num_knobs2);foo++) {

	  if(!strcmp(bar->name,baz->name)) {
#ifdef VERBOSE_CONSOLE
		printf("Overridden---> %s\n",bar->name);
#endif
		bar->knob_val = baz->knob_val;
		bar->knob_orig = baz->knob_orig;
		
	  }

	  baz++;

	}

	bar++;

  }



  /* add values present in rc.conf but not in the defaults file */
  total_nodes=(*num_knobs);
  baz=*rc_knobs2;

  for(j=0;j<(*num_knobs2);j++) {

	node_present=0;
	bar=*rc_knobs;
	for(k=0;k<(*num_knobs);k++) {

	  if(!strncmp(baz->name,bar->name,255)) node_present=1;
	  bar++;

	}

	if(node_present==0) {

	  memcpy((rc_knobs_final+total_nodes),baz,sizeof(RC_NODE));
	  total_nodes++;

	}

	baz++;

  }

  *num_knobs=total_nodes;
  *rc_knobs=rc_knobs_final;

  /* merge strings */
  rc_str_final=malloc( ((*num_str)+(*num_str2)) *sizeof(RC_NODE));
  memcpy(rc_str_final,(*rc_str),(*num_str)*sizeof(RC_NODE));

  baz=*rc_str2;
  bar=rc_str_final;


  for(j=0;j<(*num_str);j++) {

	baz=*rc_str2;
	for(foo=0;foo<(*num_str2);foo++) {

	  if(!strcmp(bar->name,baz->name)) {

#ifdef VERBOSE_CONSOLE
		printf("Overridden---> %s\n",bar->name);
#endif
		memset(bar->value,0,255);
		strncpy(bar->value,baz->value,255);

		memset(bar->orig,0,255);
		strncpy(bar->orig,baz->orig,255);

	  }

	  baz++;

	}

	bar++;

  }

  /* add values present in rc.conf but not in the defaults file */
  total_nodes=(*num_str);

  baz=*rc_str2;
  for(j=0;j<(*num_str2);j++) {

	node_present=0;
	bar=*rc_str;

	for(k=0;k<(*num_str);k++) {

	  if(!strncmp(baz->name,bar->name,255)) node_present=1;
	  bar++;

	}

	if(node_present==0) {

#ifdef VERBOSE_CONSOLE
	  printf("Adding new string: %s\n",baz->name);
#endif
	  memcpy((rc_str_final+total_nodes),baz,sizeof(RC_NODE));
	  total_nodes++;
	}

	baz++;

  }

  *num_str=total_nodes;
  *rc_str=rc_str_final;

  /* everything went ok */
  return 0;

}

/* This sort routine sucks, so, please, avoid the flames :-) */
void
list_sort(RC_NODE **rc_data, int num)
{
  int i,j,retval,changed;
  RC_NODE *list;
  RC_NODE temp;

  list=(*rc_data);

  do {

	changed=0;
	for(i=0;i<num-1;i++) {
	  for(j=i;j<num-1-i;j++) {
		retval=strncmp(list[j].name,list[j+1].name,255);	
		if(retval>=1) {
#ifdef VERBOSE_CONSOLE
		  printf("Sorting: %s (%x) > %s (%x)\n",list[j].name,list+j,list[j+1].name,list+j+1);
#endif
		  memcpy(&temp,(list+j),sizeof(RC_NODE));
		  memcpy((list+j),(list+j+1),sizeof(RC_NODE));
		  memcpy((list+j+1),&temp,sizeof(RC_NODE));
		  changed=1;
		}		
	  }
	}

  } while(changed==1);

}
