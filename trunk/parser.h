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

#ifndef PARSER_H
#define PARSER_H

typedef struct rc_node {

  char name[255];
  char value[255];
  char orig[255];
  char comment[255];
  int user_comment;
  int user_added;
  int modified;
  int knob;
  int knob_val;		
  int knob_orig;
  struct rc_node *next;
	
} RC_NODE;


						
#if defined(__FreeBSD__) || defined(__DragonFly__)
#define KNOB_YES "\"YES\""
#define KNOB_NO "\"NO\""
#elif defined(__NetBSD__)
#define KNOB_YES "YES"
#define KNOB_NO "NO"
#endif

#define KNOB_IS_YES 1
#define KNOB_IS_NO 0
#define IS_KNOB 1
#define NOT_KNOB 0
#define MODIFIED_NO 0
#define MODIFIED_YES 1

#define USER_ADDED_YES 0xEE
#define USER_ADDED_NO 0

int build_list(char *,int,RC_NODE **,int *,RC_NODE **,int *);
int merge_lists(RC_NODE **,int *,RC_NODE **,int *,RC_NODE **,int *,RC_NODE **,int *);

void list_sort(RC_NODE **, int);

#endif
