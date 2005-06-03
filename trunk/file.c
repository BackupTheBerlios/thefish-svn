/*
  Copyright (c) 2005, Miguel Mendez. All rights reserved.

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

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <time.h>
extern char *tzname[2];

#include "thefish.h"
#include "parser.h"
#include "file.h"

/*
 * This function saves changes to $FISH_RC or 
 * /etc/rc.conf
 * Returns 0 on success, -1 on error.
 */
int
save_changes(RC_NODE *r_ptr, int r_num, RC_NODE *s_ptr, int s_num)
{

  int i, not_committed;
  RC_NODE *work;
  FILE *fd;
  char *rc_file;
  char fish_header[255];
  char time_buf[255];
  time_t comm_time;

  not_committed = 0;

  work = r_ptr;

  rc_file = getenv("FISH_RC");

  if(rc_file != NULL) {

    fd = fopen(rc_file, "a");

  } else { 

    fd = fopen(RC_FILE, "a");

  }

  if(fd == NULL) {

    return(-1);

  }

  comm_time = time(NULL);
  ctime_r(&comm_time, time_buf);
  snprintf(fish_header, 255, "\n# The Fish generated deltas - %s", time_buf);
  fprintf(fd,fish_header);

  /* modified knobs */
  for(i=0; i<=r_num; i++) {

    if(work->modified == MODIFIED_YES) {

      if(work->user_comment == 0) {

	fprintf(fd,"%s=%s\n", work->name,
		work->knob_val == KNOB_IS_YES ? KNOB_YES : KNOB_NO);

      } else {

	fprintf(fd,"%s=%s\t# %s\n", work->name,
		work->knob_val == KNOB_IS_YES ? KNOB_YES : KNOB_NO,
		work->comment);

      }

      work->user_comment = 0;
      work->user_added = USER_ADDED_NO;
      work->modified = MODIFIED_NO;
      work->knob_orig = work->knob_val;

    }

    work++;

  } 

  /* modified strings */
  work = s_ptr;

  for(i=0; i<=s_num; i++) {

    if(work->modified == MODIFIED_YES) {

      if(work->user_comment == 0) {

	fprintf(fd,"%s=%s\n", work->name, work->value);

      } else {

	fprintf(fd,"%s=%s\t# %s\n", work->name, work->value,
		work->comment);

      }

      work->user_comment = 0;
      work->modified = MODIFIED_NO;
      work->user_added = USER_ADDED_NO;
      strncpy(work->orig, work->value, 255);

    }

    work++;

  }

  fclose(fd);
  return(0);
  
}
