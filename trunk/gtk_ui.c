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
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <time.h>
extern char *tzname[2];

#include <gtk/gtk.h>

#include "parser.h"
#include "gtk_ui.h"
#include "thefish.h"
#include "icon16.xpm"
#include "icon32.xpm"
#include "icon48.xpm"
#include "icon64.xpm"

#define RC_DEFAULTS_FILE "/etc/defaults/rc.conf"
#define RC_FILE "/etc/rc.conf"
#define UNCHANGED_ICON GTK_STOCK_REMOVE

/* Function prototypes */
gint delete_event( GtkWidget *, GdkEvent *, gpointer);
void destroy( GtkWidget *, gpointer);
void commit_pressed(GtkWidget *, gpointer);
void quit_pressed(GtkWidget *, gpointer);
void about_pressed(GtkWidget *, gpointer);
void add_pressed(GtkWidget *, gpointer);
void radio_yes_pressed(GtkWidget *, gpointer);
void radio_no_pressed(GtkWidget *, gpointer);
void entry_modified(GtkWidget *, gpointer);
void add_yes_pressed(GtkWidget *, gpointer);
void add_no_pressed(GtkWidget *, gpointer);
void save_geometry(void);

/* Some defines here */
#define IS_DIRTY 1
#define NOT_DIRTY 0

/* These need to be accessed from the callback functions too */

GtkWidget **radio_yes1, **radio_no1;
GSList **group1;
int r_num;
RC_NODE *r_ptr;

GtkWidget **knob_label;
GtkWidget **str_label;
GtkWidget **str_entry;
GtkTooltips **knob_tips;
GtkTooltips **str_tips;
GtkWidget **knob_status;
GtkWidget **str_status;

int s_num;
RC_NODE *s_ptr;

int dirty;

GtkWidget *mytable1;
GtkWidget *mytable2;
GtkWidget *msg_window;
GtkWidget *msg_button1;
GtkWidget *msg_button2;
GtkWidget *quit_yes_button;
GtkWidget *quit_no_button;
GtkWidget *msg_hsep;
GtkWidget *msg_vsep;
GtkWidget *msg_vbox;
GtkWidget *msg_label;
GtkWidget *msg_pixmap;
GtkWidget *msg_hbox;
GtkWidget *popup_window;
GtkWidget *popup_button;
GtkWidget *popup_hsep;
GtkWidget *popup_vbox;
GtkWidget *pixmap1;

/* Add entry widgets */

GtkWidget * add_window;
GtkWidget * add_yes_button;
GtkWidget * add_no_button;
GtkWidget * add_hsep;
GtkWidget * add_vbox;
GtkWidget * add_hbutton;
GtkWidget * add_entry1;
GtkWidget * add_entry2;
GtkWidget * add_entry3;
GtkWidget * add_frame1;
GtkWidget * add_frame2;
GtkWidget * add_frame3;

/* The quit confirm window */
GtkWidget * quit_hbutton;

/* Avoid trying to create the same window twice */
int commit_win_up;
int about_win_up;
int add_win_up;
int quit_win_up;

GtkWidget *commit_button;
GtkWidget *window;
GtkWidget *myviewport1;
GtkWidget *myviewport2;
GtkWidget *mynotebook;

/* Window geometry */
/* Deprecated for file, we
 * now use human readable data
 */
int oldsize[2];

/* This funcion creates the main UI */
int
create_gtk_ui(RC_NODE *rc_knobs,int num_knobs,RC_NODE *rc_strings,int num_str)
{

  RC_NODE *work;

  GtkWidget *quit_button;
  GtkWidget *about_button;
  GtkWidget *about_icon;
  GtkWidget *about_label;
  GtkWidget *about_hbox;
  GtkWidget *about_align;
  GtkWidget *add_button;
  GtkWidget *h_buttons;

  /* Knobs */
  GtkWidget *vbox1;
  GtkWidget *hseparator1;

  /* Strings */
  GtkWidget *vbox2;
  GtkWidget *hseparator2;

  /* Static widgets */
  GtkWidget *tab_bools;
  GtkWidget *tab_str;
  GtkTooltips *commit_tip;
  GtkTooltips *add_tip;
  GtkTooltips *about_tip;
  GtkTooltips *quit_tip;

  /* For the window icon */
  GdkPixbuf *icon16_pixbuf;
  GdkPixbuf *icon32_pixbuf;
  GdkPixbuf *icon48_pixbuf;
  GdkPixbuf *icon64_pixbuf;
  GList *icon_list = NULL;

  GtkWidget *scrolled_window1;
  GtkWidget *scrolled_window2;

  int i;
  char *homedir;
  char temp[FILENAME_MAX];
  int fd;
  FILE *fp;

  commit_win_up=FALSE;
  add_win_up=FALSE;
  about_win_up=FALSE;
  quit_win_up=FALSE;	
  r_ptr=rc_knobs;
  s_ptr=rc_strings;
  dirty=NOT_DIRTY;

  /* Define main window */
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_resizable(GTK_WINDOW(window),TRUE);
  gtk_window_set_title(GTK_WINDOW(window),"The Fish");


  /* Set the size.
   * We're now using human readable data, handle the migration
   * transparently for the user.
   */
  homedir=getenv("HOME");

  if(homedir!=NULL) {

    snprintf(temp,FILENAME_MAX,"%s/%s",homedir,".thefishrc");
    fd=open(temp,O_RDONLY,0);

    if(fd!=-1) {

      i=lseek(fd,0,SEEK_END);
      lseek(fd,0,SEEK_SET);

      if(i==sizeof(oldsize)) {

	read(fd,&oldsize[0],sizeof(oldsize));
	close(fd);

      } else {

	fp=fdopen(fd,"r");
	fscanf(fp,"geometry=%i,%i",&oldsize[0],&oldsize[1]);
	fclose(fp);

      }

    } else  {
      /* Set some default values */
      oldsize[0]=400;
      oldsize[1]=480;

    }

  }

  gtk_window_set_default_size(GTK_WINDOW(window),oldsize[0],oldsize[1]);

  /* Set the icon */
  icon16_pixbuf=gdk_pixbuf_new_from_xpm_data((const char **)icon16);
  icon32_pixbuf=gdk_pixbuf_new_from_xpm_data((const char **)icon32);
  icon48_pixbuf=gdk_pixbuf_new_from_xpm_data((const char **)icon48);
  icon64_pixbuf=gdk_pixbuf_new_from_xpm_data((const char **)icon64);

  g_list_append(icon_list,icon16_pixbuf);
  g_list_append(icon_list,icon32_pixbuf);
  g_list_append(icon_list,icon48_pixbuf);	
  g_list_append(icon_list,icon64_pixbuf);

  gtk_window_set_icon(GTK_WINDOW(window),icon64_pixbuf);
  gtk_window_set_default_icon_list(icon_list);

  g_list_free (icon_list);

  /* Basic buttons */
  commit_button = gtk_button_new_from_stock(GTK_STOCK_SAVE);
  quit_button = gtk_button_new_from_stock(GTK_STOCK_QUIT);

  about_button = gtk_button_new();
  about_icon = gtk_image_new_from_stock(GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  about_label = gtk_label_new("About");
  about_hbox = gtk_hbox_new(FALSE, 2);
  about_align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
  gtk_box_pack_start(GTK_BOX(about_hbox), about_icon, FALSE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(about_hbox), about_label, FALSE, FALSE, 0);
  gtk_container_add(GTK_CONTAINER(about_button), about_align);
  gtk_container_add(GTK_CONTAINER(about_align), about_hbox);

  add_button = gtk_button_new_from_stock(GTK_STOCK_ADD);

  /* Button tooltips */
  commit_tip=gtk_tooltips_new();
  gtk_tooltips_set_tip(commit_tip,commit_button,"Save changes","");
  gtk_tooltips_enable(commit_tip);

  add_tip=gtk_tooltips_new();
  gtk_tooltips_set_tip(add_tip,add_button,"Add a new entry","");
  gtk_tooltips_enable(add_tip);

  about_tip=gtk_tooltips_new();
  gtk_tooltips_set_tip(about_tip,about_button,"About The Fish","");
  gtk_tooltips_enable(about_tip);

  quit_tip=gtk_tooltips_new();
  gtk_tooltips_set_tip(quit_tip,quit_button,"Exit The Fish","");
  gtk_tooltips_enable(quit_tip);

  /* Nothing to save yet, so disable the button */
  gtk_widget_set_sensitive(commit_button,FALSE);

  h_buttons=gtk_hbutton_box_new();

  gtk_box_pack_start(GTK_BOX(h_buttons), commit_button, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(h_buttons), add_button, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(h_buttons), about_button, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(h_buttons), quit_button, FALSE, FALSE, 0);

  /* Callback handlers */
  g_signal_connect(GTK_OBJECT(window), "delete_event",\
		   GTK_SIGNAL_FUNC(delete_event), NULL);

  g_signal_connect(GTK_OBJECT(window), "destroy",\
		   GTK_SIGNAL_FUNC(destroy), NULL);

  g_signal_connect(GTK_OBJECT(commit_button), "clicked",\
		   GTK_SIGNAL_FUNC(commit_pressed), NULL);

  g_signal_connect(GTK_OBJECT(quit_button), "clicked",\
		   GTK_SIGNAL_FUNC(quit_pressed), NULL);

  g_signal_connect(GTK_OBJECT(about_button), "clicked",\
		   GTK_SIGNAL_FUNC(about_pressed), NULL);

  g_signal_connect(GTK_OBJECT(add_button), "clicked",\
		   GTK_SIGNAL_FUNC(add_pressed), NULL);

  mytable1=gtk_table_new(num_knobs*2,4,FALSE);
  gtk_table_set_col_spacings(GTK_TABLE(mytable1),10);

  /* Dynamic stuff generated at run time */
  /* First the knobs */
  work=rc_knobs;

  /* Reserve memory for our pointer arrays */
  knob_label=malloc(2*num_knobs*sizeof(GtkWidget *));
  radio_yes1=malloc(2*num_knobs*sizeof(GtkWidget *));
  radio_no1=malloc(2*num_knobs*sizeof(GtkWidget *));
  group1=malloc(2*num_knobs*sizeof(GtkWidget *));
  knob_tips=malloc(2*num_knobs*sizeof(GtkTooltips *));

  knob_status=malloc(2*num_knobs*sizeof(GtkWidget *));

  vbox1 = gtk_vbox_new (FALSE, 2);

  r_num=num_knobs;
  s_num=num_str;

  for(i=0;i<num_knobs;i++) {

    /* No user comments yet */
    (rc_knobs+i)->user_comment=0;

    radio_yes1[i]=gtk_radio_button_new_with_label(NULL,"yes");
    group1[i]=gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_yes1[i]));
    radio_no1[i]=gtk_radio_button_new_with_label(group1[i],"no");

    if((rc_knobs+i)->knob_val==KNOB_IS_NO) {

      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_no1[i]),TRUE);

    } 

    g_signal_connect(GTK_OBJECT(radio_yes1[i]), "pressed",\
		     GTK_SIGNAL_FUNC(radio_yes_pressed), NULL);

    g_signal_connect(GTK_OBJECT(radio_no1[i]), "pressed",\
		     GTK_SIGNAL_FUNC(radio_no_pressed), NULL);


    knob_status[i]=gtk_image_new_from_stock(UNCHANGED_ICON,GTK_ICON_SIZE_MENU);

    knob_label[i]=gtk_button_new_with_label(work->name);
    gtk_button_set_relief(GTK_BUTTON(knob_label[i]),(GtkReliefStyle) GTK_RELIEF_NONE);

    knob_tips[i]=gtk_tooltips_new();
    gtk_tooltips_set_tip(knob_tips[i],knob_label[i],(rc_knobs+i)->comment,"");
    gtk_tooltips_enable(knob_tips[i]);

    gtk_table_attach(GTK_TABLE(mytable1), GTK_WIDGET(knob_status[i]),
		     0,1, i, i+1, 0, GTK_EXPAND,0,0);
    gtk_table_attach(GTK_TABLE(mytable1), GTK_WIDGET(knob_label[i]), 
		     1,2, i, i+1, 0, GTK_EXPAND,0,0);
    gtk_table_attach(GTK_TABLE(mytable1), GTK_WIDGET(radio_yes1[i]), 
		     2,3, i, i+1, 0, GTK_EXPAND,0,0);
    gtk_table_attach(GTK_TABLE(mytable1), GTK_WIDGET(radio_no1[i]), 
		     3, 4,i,i+1, 0, GTK_EXPAND,0,0);

    work++;

  }


  hseparator1 = gtk_hseparator_new ();	

  /* New code added on Jan 25 2003
   * We use a scrolled window for both the knobs 
   * and the strings
   */
  scrolled_window1=gtk_scrolled_window_new(NULL, NULL);
  gtk_container_set_border_width(GTK_CONTAINER(scrolled_window1),10);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window1),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window1)
					,mytable1);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);

  tab_bools=gtk_label_new("Knobs");
  tab_str=gtk_label_new("Strings");

  mynotebook=gtk_notebook_new();

  gtk_notebook_append_page(GTK_NOTEBOOK(mynotebook),scrolled_window1,tab_bools);

  gtk_box_pack_start(GTK_BOX(vbox1), mynotebook, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox1), hseparator1, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox1), h_buttons, FALSE, FALSE, 0);
  gtk_box_set_homogeneous(GTK_BOX(vbox1),FALSE);
  gtk_container_add(GTK_CONTAINER(window), vbox1);

  gtk_widget_show(mynotebook);
  gtk_widget_show(vbox1);

  for(i=0;i<num_knobs;i++) {

    gtk_widget_show(knob_status[i]);
    gtk_widget_show(knob_label[i]);
    gtk_widget_show(radio_yes1[i]);
    gtk_widget_show(radio_no1[i]);

  }

  /* Now, the strings */
  work=rc_strings;

  mytable2=gtk_table_new(num_knobs*2,3,FALSE);
  gtk_table_set_col_spacings(GTK_TABLE(mytable2),10);
  gtk_table_set_homogeneous(GTK_TABLE(mytable2),FALSE);

  /* Allocate memory for our pointer arrays */
  str_label=malloc(2*num_str*sizeof(GtkWidget *));
  str_entry=malloc(2*num_str*sizeof(GtkWidget *));
  str_tips=malloc(2*num_str*sizeof(GtkTooltips *));	
  str_status=malloc(2*num_str*sizeof(GtkWidget *));

  vbox2 = gtk_vbox_new(FALSE, 2);

  for(i=0;i<num_str;i++) {

    /* No user comments yet */
    (rc_strings+i)->user_comment=0;
	
    str_label[i]=gtk_button_new_with_label(work->name);
    gtk_button_set_relief(GTK_BUTTON(str_label[i]),(GtkReliefStyle) GTK_RELIEF_NONE);

    str_status[i]=gtk_image_new_from_stock(UNCHANGED_ICON,GTK_ICON_SIZE_MENU);
    str_tips[i]=gtk_tooltips_new();
    gtk_tooltips_set_tip(str_tips[i],str_label[i],(rc_strings+i)->comment,"");
    gtk_tooltips_enable(str_tips[i]);

    str_entry[i]=gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(str_entry[i]),255);
    gtk_entry_set_text(GTK_ENTRY(str_entry[i]),work->value);

    g_signal_connect(GTK_OBJECT(str_entry[i]), "changed",\
		     GTK_SIGNAL_FUNC(entry_modified), NULL);


    gtk_table_attach(GTK_TABLE(mytable2), GTK_WIDGET(str_status[i]),
		     0,1, i, i+1, 0, GTK_EXPAND,0,0);
    gtk_table_attach(GTK_TABLE(mytable2), GTK_WIDGET(str_label[i])
		     , 1,2, i, i+1,NULL, NULL,0,0);
    gtk_table_attach_defaults(GTK_TABLE(mytable2), GTK_WIDGET(str_entry[i])
			      , 2,3, i, i+1);
    work++;

  }

  hseparator2 = gtk_hseparator_new();

  scrolled_window2=gtk_scrolled_window_new(NULL, NULL);
  gtk_container_set_border_width(GTK_CONTAINER(scrolled_window2),10);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window2),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window2)
					,mytable2);

  gtk_notebook_append_page(GTK_NOTEBOOK(mynotebook),scrolled_window2,tab_str);

  for(i=0;i<num_str;i++) {
	
    gtk_widget_show(str_label[i]);
    gtk_widget_show(str_entry[i]);
    gtk_widget_show(str_status[i]);

  }

  /* Finally, the static widgets */
  gtk_widget_show(hseparator1);
  gtk_widget_show(h_buttons);
  gtk_widget_show(commit_button);
  gtk_widget_show(quit_button);
  gtk_widget_show(about_button);
  gtk_widget_show(about_label);
  gtk_widget_show(about_hbox);
  gtk_widget_show(about_icon);
  gtk_widget_show(about_align);
  gtk_widget_show(add_button);
  gtk_widget_show (scrolled_window2);
  gtk_widget_show (mytable2);
  gtk_widget_show (scrolled_window1);
  gtk_widget_show (mytable1);
  gtk_widget_show(window);

  /* Let the show begin */
  gtk_main ();

  return 0;
}

/* CALLBACKS */

gint 
delete_event( GtkWidget *widget, GdkEvent *event, gpointer data)
{
  GtkWidget *dialog;
  gint result;

  if(dirty>0) {

    dialog = gtk_message_dialog_new (GTK_WINDOW(window),
				     GTK_DIALOG_DESTROY_WITH_PARENT,
				     GTK_MESSAGE_QUESTION,
				     GTK_BUTTONS_YES_NO,
				     "There are unsaved changes, quit anyway?");
    result=gtk_dialog_run (GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

  } else {

    return FALSE;

  }

  if(result==GTK_RESPONSE_YES) {

    return FALSE;

  } else {

    return TRUE;

  }

}

void 
destroy( GtkWidget *widget, gpointer data)
{
  save_geometry();
  gtk_main_quit();
}

/* Warn the user if she has unsaved changes */
void 
quit_pressed( GtkWidget *widget, gpointer data)
{

  GtkWidget *dialog;
  gint result;


  if(dirty>0) {

    dialog = gtk_message_dialog_new (GTK_WINDOW(window),
				     GTK_DIALOG_DESTROY_WITH_PARENT,
				     GTK_MESSAGE_QUESTION,
				     GTK_BUTTONS_YES_NO,
				     "There are unsaved changes, quit anyway?");
    result=gtk_dialog_run (GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    if(result==GTK_RESPONSE_YES) {

      save_geometry();
      gtk_main_quit();

    }

  } else {

    save_geometry();
    gtk_main_quit();

  }

}

/* This function saves changes to $FISH_RC or 
 * /etc/rc.conf
 */
void 
commit_pressed( GtkWidget *widget, gpointer data)
{

  GtkWidget *dialog; /* for the GTK2 dialogs */

  int i,not_committed;
  RC_NODE *work;
  FILE *fd;
  char *rc_file;
  char fish_header[255];
  char time_buf[255];
  time_t comm_time;

  not_committed=0;

  if(commit_win_up==FALSE) {

    if(dirty>0) {

      work=r_ptr;

      rc_file=getenv("FISH_RC");
      if(rc_file!=NULL) {

	fd=fopen(rc_file,"a");

      } else { 

	fd=fopen(RC_FILE,"a");

      }

      if(fd==NULL) {

	not_committed=1;			

      }

      if(not_committed==0) {

	comm_time=time(NULL);
	ctime_r(&comm_time,time_buf);
	snprintf(fish_header,255,"\n#The Fish generated deltas - %s",time_buf);
	fprintf(fd,fish_header);

	/* modified knobs */
	for(i=0;i<=r_num;i++) {

	  if(work->modified==MODIFIED_YES) {

	    if(work->user_comment==0) {

	      fprintf(fd,"%s=%s\n",work->name,\
		      work->knob_val == KNOB_IS_YES ? KNOB_YES : KNOB_NO);

	    } else {

	      fprintf(fd,"%s=%s\t# %s\n",work->name,\
		      work->knob_val == KNOB_IS_YES ? KNOB_YES : KNOB_NO,\
		      work->comment);

	    }

	    work->user_comment=0;
	    work->user_added=USER_ADDED_NO;
	    work->modified=MODIFIED_NO;
	    work->knob_orig=work->knob_val;
	    gtk_image_set_from_stock(GTK_IMAGE(knob_status[i]),
				     UNCHANGED_ICON,GTK_ICON_SIZE_MENU);
	  }

	  work++;

	} 

	/* modified strings */
	work=s_ptr;
	for(i=0;i<=s_num;i++) {

	  if(work->modified==MODIFIED_YES) {

	    if(work->user_comment==0) {

	      fprintf(fd,"%s=%s\n",work->name,work->value);

	    } else {

	      fprintf(fd,"%s=%s\t# %s\n",work->name,work->value,\
		      work->comment);

	    }

	    work->user_comment=0;
	    work->modified=MODIFIED_NO;
	    work->user_added=USER_ADDED_NO;
	    strncpy(work->orig,work->value,255);
	    gtk_image_set_from_stock(GTK_IMAGE(str_status[i]),
				     UNCHANGED_ICON,GTK_ICON_SIZE_MENU);
	  }

	  work++;

	}  

	fclose(fd);

      }

      /* Pop up window */
      if(not_committed==1) {

	dialog = gtk_message_dialog_new (GTK_WINDOW(window),
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_CLOSE,
					 "Can't open '%s' for writing. Changes not saved",
					 rc_file!=NULL ? rc_file : RC_FILE);
      } else {

	dialog = gtk_message_dialog_new (GTK_WINDOW(window),
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_INFO,
					 GTK_BUTTONS_OK,
					 "Changes saved");
      }

      gtk_dialog_run (GTK_DIALOG(dialog));
      gtk_widget_destroy (dialog);

    } else {

      dialog = gtk_message_dialog_new (GTK_WINDOW(window),
				       GTK_DIALOG_DESTROY_WITH_PARENT,
				       GTK_MESSAGE_INFO,
				       GTK_BUTTONS_OK,
				       "There are no unsaved changes");

      gtk_dialog_run (GTK_DIALOG(dialog));
      gtk_widget_destroy (dialog);

    }

    if(not_committed==0) {

      dirty=NOT_DIRTY;
      gtk_widget_set_sensitive(commit_button,FALSE);

    }
  }

}

/* This callback is called whenever a 'yes'
 * radio button is pressed. It has to find
 * out which widget generated the signal and
 * modify the relevant RC_NODE
 */
void 
radio_yes_pressed( GtkWidget *widget, gpointer data)
{
  int i;
  RC_NODE *work;

  work=r_ptr;

  for(i=0;i<=r_num;i++) {

    if(widget==radio_yes1[i]) {

      break;

    }

#ifdef VERBOSE_CONSOLE
    printf("Pointer comparison: %x %x\n",widget,radio_yes1[i]);
#endif
    work++;
  }

#ifdef VERBOSE_CONSOLE
  printf("You pressed YES on widget %i : %s\n",i,work->name);	
#endif

  work->knob_val=KNOB_IS_YES;

  if(work->knob_orig==KNOB_IS_NO) {

    work->modified=MODIFIED_YES;
    dirty++;
    gtk_widget_set_sensitive(commit_button,TRUE);
    gtk_image_set_from_stock(GTK_IMAGE(knob_status[i]),
			     GTK_STOCK_SAVE,GTK_ICON_SIZE_MENU);


  } else if(work->user_added==USER_ADDED_NO) {

    work->modified=MODIFIED_NO;
    gtk_image_set_from_stock(GTK_IMAGE(knob_status[i]),
			     UNCHANGED_ICON,GTK_ICON_SIZE_MENU);
    if(dirty>0) dirty--;
    if(dirty==NOT_DIRTY) gtk_widget_set_sensitive(commit_button,FALSE);
	
  }

}

/* Same as above for 'no' buttons */
void 
radio_no_pressed( GtkWidget *widget, gpointer data)
{

  int i;
  RC_NODE *work;

  work=r_ptr;

  for(i=0;i<=r_num;i++) {

    if(widget==radio_no1[i]) {

      break;

    }

#ifdef VERBOSE_CONSOLE
    printf("Pointer comparison: %x %x\n",widget,radio_yes1[i]);
#endif
    work++;

  }

#ifdef VERBOSE_CONSOLE
  printf("You pressed NO on widget %i : %s\n",i,work->name);	
#endif

  work->knob_val=KNOB_IS_NO;
  if(work->knob_orig==KNOB_IS_YES) {

    work->modified=MODIFIED_YES;
    dirty++;
    gtk_widget_set_sensitive(commit_button,TRUE);
    gtk_image_set_from_stock(GTK_IMAGE(knob_status[i]),
			     GTK_STOCK_SAVE,GTK_ICON_SIZE_MENU);

  } else if(work->user_added==USER_ADDED_NO) {

    work->modified=MODIFIED_NO;
    if(dirty>0) dirty--;
    if(dirty==NOT_DIRTY) gtk_widget_set_sensitive(commit_button,FALSE);
    gtk_image_set_from_stock(GTK_IMAGE(knob_status[i]),
			     UNCHANGED_ICON,GTK_ICON_SIZE_MENU);
  }

}

/* If an string is modified by the user, this 
 * function will be called. It finds which widget
 * has been modified and syncs the new entry value
 * with the value stored in the RC_NODE structure.
 */
void 
entry_modified(GtkWidget *widget, gpointer data)
{
  int i;
  RC_NODE *work;
  char *entry_val;

  work=s_ptr;

  for(i=0;i<=s_num;i++) {

    if(widget==str_entry[i]) {
      break;

    }

#ifdef VERBOSE_CONSOLE
    printf("Pointer comparison: %x %x\n",widget,radio_yes1[i]);
#endif
    work++;

  }

  entry_val=(char *) gtk_entry_get_text(GTK_ENTRY(widget));

#ifdef VERBOSE_CONSOLE
  printf("You modified widget %s (%i) : %s -> %s\n",work->name,i,\
	 work->value,entry_val);	
#endif

  strncpy(work->value,entry_val,255);
  if(!strncmp(work->value,work->orig,255) && work->user_added==USER_ADDED_NO) {

    work->modified=MODIFIED_NO;
    if(dirty>0) dirty--;
    if(dirty==NOT_DIRTY) gtk_widget_set_sensitive(commit_button,FALSE);
    gtk_image_set_from_stock(GTK_IMAGE(str_status[i]),
			     UNCHANGED_ICON,GTK_ICON_SIZE_MENU);
  } else {

    work->modified=MODIFIED_YES;
    dirty++;
    gtk_widget_set_sensitive(commit_button,TRUE);
    gtk_image_set_from_stock(GTK_IMAGE(str_status[i]),
			     GTK_STOCK_SAVE,GTK_ICON_SIZE_MENU);
  }

}

/* The usual 'About' window */
void 
about_pressed(GtkWidget *widget, gpointer data)
{

  GtkWidget *dialog;

  dialog = gtk_message_dialog_new (GTK_WINDOW(window),
				   GTK_DIALOG_DESTROY_WITH_PARENT,
				   GTK_MESSAGE_INFO,
				   GTK_BUTTONS_OK,
				   "The Fish "
				   THE_FISH_VERSION
				   "\nCopyright (c) 2002-2004, "
				   "Miguel Mendez\nE-Mail: <flynn@energyhq.es.eu.org>\n"
				   "http://www.energyhq.es.eu.org/thefish.html\n");

  gtk_dialog_run (GTK_DIALOG(dialog));
  gtk_widget_destroy (dialog);

}

/* The user has pressed the add button. This
 * function creates the 'add entry' window,
 * with Name, Value and Comment entries 
 */
void 
add_pressed(GtkWidget * widget, gpointer data)
{
  if(add_win_up==FALSE) {

    add_win_up=TRUE;

    add_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position (GTK_WINDOW (add_window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(add_window),TRUE);
    gtk_window_set_title(GTK_WINDOW(add_window),"Add new entry");

    add_yes_button = gtk_button_new_from_stock(GTK_STOCK_APPLY);

    g_signal_connect(GTK_OBJECT(add_yes_button), "clicked",\
		     GTK_SIGNAL_FUNC(add_yes_pressed), NULL);

    add_no_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);

    g_signal_connect(GTK_OBJECT(add_no_button), "clicked",\
		     GTK_SIGNAL_FUNC(add_no_pressed), NULL);

    add_hsep = gtk_hseparator_new ();
    add_vbox = gtk_vbox_new (FALSE, 0);

    add_hbutton=gtk_hbutton_box_new();

    gtk_box_pack_start(GTK_BOX(add_hbutton), add_yes_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(add_hbutton), add_no_button, FALSE, FALSE, 0);

    add_frame1=gtk_frame_new("Name");
    add_entry1=gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(add_entry1),255);

    add_frame2=gtk_frame_new("Value");
    add_entry2=gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(add_entry2),255);
    gtk_entry_set_text(GTK_ENTRY(add_entry2),"\"\"");

    add_frame3=gtk_frame_new("Optional Comment");
    add_entry3=gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(add_entry3),255);

    add_hsep = gtk_hseparator_new();

    gtk_container_add(GTK_CONTAINER(add_frame1), add_entry1);
    gtk_container_add(GTK_CONTAINER(add_frame2), add_entry2);
    gtk_container_add(GTK_CONTAINER(add_frame3), add_entry3);

    gtk_box_pack_start(GTK_BOX(add_vbox), add_frame1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(add_vbox), add_frame2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(add_vbox), add_frame3, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(add_vbox), add_hsep, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(add_vbox), add_hbutton, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(add_window), add_vbox);

    gtk_widget_show(add_frame1);
    gtk_widget_show(add_entry1);
    gtk_widget_show(add_frame2);
    gtk_widget_show(add_entry2);
    gtk_widget_show(add_frame3);
    gtk_widget_show(add_entry3);
    gtk_widget_show(add_yes_button);
    gtk_widget_show(add_no_button);
    gtk_widget_show(add_hsep);
    gtk_widget_show(add_vbox);
    gtk_widget_show(add_hbutton);
    gtk_widget_show(add_window);

  }
}

/* User wants to add, we need to get the new values
 * and do some sanity checks.
 */
void 
add_yes_pressed(GtkWidget * widget, gpointer data)
{

  GtkWidget *dialog;
  int i,dupe;
  char *new_value;
  char *new_name;
  char *new_comment;

  add_win_up=FALSE;
  dupe=0;
  new_name=(char *) gtk_entry_get_text(GTK_ENTRY(add_entry1));
  new_value=(char *) gtk_entry_get_text(GTK_ENTRY(add_entry2));
  new_comment=(char *) gtk_entry_get_text(GTK_ENTRY(add_entry3));

  /* Check for duplicate entries */
  for(i=0;i<r_num;i++) {

    if(!strncmp(r_ptr[i].name,new_name,255)) {

      dupe=1;
      break;

    }

  }

  for(i=0;i<s_num;i++) {

    if(!strncmp(s_ptr[i].name,new_name,255)) {

      dupe=1;
      break;

    }

  }

  if(strlen(new_name)==0 || strlen(new_value)==0) {

    /* Show warning dialog */
    dialog = gtk_message_dialog_new (GTK_WINDOW(window),
				     GTK_DIALOG_DESTROY_WITH_PARENT,
				     GTK_MESSAGE_ERROR,
				     GTK_BUTTONS_CLOSE,
				     "name and value cannot be empty");

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy (dialog);

  } else if(new_value[0]!='"'||new_value[strlen(new_value)-1]!='"') {

    /* Show warning dialog */
    dialog = gtk_message_dialog_new (GTK_WINDOW(window),
				     GTK_DIALOG_DESTROY_WITH_PARENT,
				     GTK_MESSAGE_ERROR,
				     GTK_BUTTONS_CLOSE,
				     "value must begin and end with \".");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

  } else if(dupe==1) {

#ifdef VERBOSE_CONSOLE	
    printf("Duplicated entry\n");
#endif		

    /* Show warning window */
    dialog = gtk_message_dialog_new (GTK_WINDOW(window),
				     GTK_DIALOG_DESTROY_WITH_PARENT,
				     GTK_MESSAGE_WARNING,
				     GTK_BUTTONS_CLOSE,
				     "Duplicated entry");

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

  } else {

#ifdef VERBOSE_CONSOLE	
    printf("Data added\n");
#endif

    /* Is a knob? */
    if( !strncasecmp(new_value,KNOB_YES,255) || !strncasecmp(new_value,KNOB_NO,255)) {

      strncpy(r_ptr[r_num].name,new_name,255);
      strncpy(r_ptr[r_num].comment,new_comment,255);
      r_ptr[r_num].user_added=USER_ADDED_YES;

      if(strlen(new_comment)>0) r_ptr[r_num].user_comment=1;

      if(!strncasecmp(new_value,KNOB_YES,255)) {

	r_ptr[r_num].knob_val=KNOB_IS_YES;
	r_ptr[r_num].knob_orig=KNOB_IS_YES;
	r_ptr[r_num].modified=MODIFIED_YES;
	dirty++;
	gtk_widget_set_sensitive(commit_button,TRUE);

      } else {

	r_ptr[r_num].knob_val=KNOB_IS_NO;
	r_ptr[r_num].knob_orig=KNOB_IS_NO;
	r_ptr[r_num].modified=MODIFIED_YES;
	dirty++;
	gtk_widget_set_sensitive(commit_button,TRUE);

      }

      gtk_notebook_set_current_page(GTK_NOTEBOOK(mynotebook),(gint) 0);

      radio_yes1[r_num]=gtk_radio_button_new_with_label(NULL,"yes");
      group1[r_num]=gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_yes1[r_num]));
      radio_no1[r_num]=gtk_radio_button_new_with_label(group1[r_num],"no");
      knob_status[r_num]=gtk_image_new_from_stock(GTK_STOCK_SAVE,GTK_ICON_SIZE_MENU);

      if(r_ptr[r_num].knob_val==KNOB_IS_NO) {

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_no1[r_num]),TRUE);

      }

      g_signal_connect(GTK_OBJECT(radio_yes1[r_num]), "pressed",\
		       GTK_SIGNAL_FUNC(radio_yes_pressed), NULL);

      g_signal_connect(GTK_OBJECT(radio_no1[r_num]), "pressed",\
		       GTK_SIGNAL_FUNC(radio_no_pressed), NULL);

      knob_label[r_num]=gtk_button_new_with_label(r_ptr[r_num].name);
      gtk_button_set_relief(GTK_BUTTON(knob_label[r_num]),\
			    (GtkReliefStyle) GTK_RELIEF_NONE);
      knob_tips[r_num]=gtk_tooltips_new();
      gtk_tooltips_set_tip(knob_tips[r_num],knob_label[r_num],r_ptr[r_num].comment,"");
      gtk_tooltips_enable(knob_tips[r_num]);

      gtk_table_attach(GTK_TABLE(mytable1), GTK_WIDGET(knob_status[r_num]),
		       0,1, r_num, r_num+1, 0, GTK_EXPAND,0,0);
      gtk_table_attach(GTK_TABLE(mytable1), GTK_WIDGET(knob_label[r_num]), 
		       1,2, r_num, r_num+1, 0, GTK_EXPAND,0,0);
      gtk_table_attach(GTK_TABLE(mytable1), GTK_WIDGET(radio_yes1[r_num]), 
		       2,3, r_num, r_num+1, 0, GTK_EXPAND,0,0);
      gtk_table_attach(GTK_TABLE(mytable1), GTK_WIDGET(radio_no1[r_num]), 
		       3, 4,r_num,r_num+1, 0, GTK_EXPAND,0,0);

      gtk_widget_show(knob_status[r_num]);
      gtk_widget_show(knob_label[r_num]);
      gtk_widget_show(radio_yes1[r_num]);
      gtk_widget_show(radio_no1[r_num]);

      r_num++;

      /* Is a string */
    } else {

      strncpy(s_ptr[s_num].name,new_name,255);
      strncpy(s_ptr[s_num].value,new_value,255);
      strncpy(s_ptr[s_num].orig,new_value,255);
      strncpy(s_ptr[s_num].comment,new_comment,255);
      s_ptr[s_num].user_added=USER_ADDED_YES;

      if(strlen(new_comment)>0) s_ptr[s_num].user_comment=1;

      s_ptr[s_num].modified=MODIFIED_YES;
      dirty++;

      gtk_widget_set_sensitive(commit_button,TRUE);

      /* Switch page */
      gtk_notebook_set_current_page(GTK_NOTEBOOK(mynotebook),(gint) 1);

      str_label[s_num]=gtk_button_new_with_label(s_ptr[s_num].name);
      gtk_button_set_relief(GTK_BUTTON(str_label[s_num]),\
			    (GtkReliefStyle) GTK_RELIEF_NONE);
      str_tips[s_num]=gtk_tooltips_new();
      gtk_tooltips_set_tip(str_tips[s_num],str_label[s_num],s_ptr[s_num].comment,"");
      gtk_tooltips_enable(str_tips[s_num]);

      str_status[s_num]=gtk_image_new_from_stock(GTK_STOCK_SAVE,GTK_ICON_SIZE_MENU);

      str_entry[s_num]=gtk_entry_new();

      gtk_entry_set_max_length(GTK_ENTRY(str_entry[s_num]),255);

      gtk_entry_set_text(GTK_ENTRY(str_entry[s_num]),s_ptr[s_num].value);

      g_signal_connect(GTK_OBJECT(str_entry[s_num]), "changed",\
		       GTK_SIGNAL_FUNC(entry_modified), NULL);

      gtk_table_attach(GTK_TABLE(mytable2), GTK_WIDGET(str_status[s_num])
		       , 0,1, s_num, s_num+1, 0, GTK_EXPAND,0,0);
      gtk_table_attach(GTK_TABLE(mytable2), GTK_WIDGET(str_label[s_num])
		       , 1,2, s_num, s_num+1,NULL, NULL,0,0);
      gtk_table_attach_defaults(GTK_TABLE(mytable2), GTK_WIDGET(str_entry[s_num])
				, 2,3, s_num, s_num+1);

      gtk_widget_show(str_status[s_num]);
      gtk_widget_show(str_entry[s_num]);
      gtk_widget_show(str_label[s_num]);

      s_num++;			

    }

    /* Close window */
    gtk_widget_hide(add_entry1);
    gtk_widget_hide(add_entry2);
    gtk_widget_hide(add_yes_button);
    gtk_widget_hide(add_no_button);
    gtk_widget_hide(add_hsep);
    gtk_widget_hide(add_vbox);
    gtk_widget_hide(add_hbutton);
    gtk_widget_hide(add_window);

    gtk_widget_destroy(add_frame1);	
    gtk_widget_destroy(add_frame2);
    gtk_widget_destroy(add_frame3);
    gtk_widget_destroy(add_yes_button);
    gtk_widget_destroy(add_no_button);
    gtk_widget_destroy(add_hsep);
    gtk_widget_destroy(add_vbox);
    gtk_widget_destroy(add_window);		
  }

}

/* User cancelled the 'add' process */
void 
add_no_pressed(GtkWidget * widget, gpointer data)
{

  gtk_widget_hide(add_entry1);
  gtk_widget_hide(add_entry2);
  gtk_widget_hide(add_entry3);
  gtk_widget_hide(add_yes_button);
  gtk_widget_hide(add_no_button);
  gtk_widget_hide(add_hsep);
  gtk_widget_hide(add_vbox);
  gtk_widget_hide(add_hbutton);
  gtk_widget_hide(add_window);

  gtk_widget_destroy(add_frame1);	
  gtk_widget_destroy(add_frame2);
  gtk_widget_destroy(add_frame3);
  gtk_widget_destroy(add_yes_button);
  gtk_widget_destroy(add_no_button);
  gtk_widget_destroy(add_hsep);
  gtk_widget_destroy(add_vbox);
  gtk_widget_destroy(add_window);

  add_win_up=FALSE;

}

/* This routine checks if the user has resized
 * the window and stores the new geometry values
 * in $HOME/.thefishrc
 */
void
save_geometry(void)
{

  int newsize[2];
  char *homedir;
  char temp[FILENAME_MAX];
  int fd;
  FILE *fp;

  gtk_window_get_size(GTK_WINDOW(window),&newsize[0],&newsize[1]);

  if(oldsize[0]!=newsize[0] || oldsize[1]!=newsize[1]) {

    homedir=getenv("HOME");
    if(homedir==NULL) return;
    snprintf(temp,FILENAME_MAX,"%s/%s",homedir,".thefishrc");
    fd=open(temp,O_WRONLY|O_CREAT|O_TRUNC,0666);
    if(fd==-1) return;
    fp=fdopen(fd,"a");
    fprintf(fp,"geometry=%i,%i\n",newsize[0],newsize[1]);
    fclose(fp);
    return;
  
  }

}
