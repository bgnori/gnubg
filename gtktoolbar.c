/*
 * gtktoolbar.c
 *
 * by Joern Thyssen <jth@gnubg.org>, 2003
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 3 or later of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: gtktoolbar.c,v 1.44 2008/03/16 09:48:36 Superfly_Jon Exp $
 */

#include "config.h"

#include <stdlib.h>

#include <gtk/gtk.h>

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "backgammon.h"
#include "gtktoolbar.h"
#include "gtkboard.h"
#include "gtkgame.h"
#include "gtk-multiview.h"
#include "gtkfile.h"
#include <glib/gi18n.h>
#include "drawboard.h"
#include "renderprefs.h"
#if USE_BOARD3D
#include "fun3d.h"
#endif

extern void NewDialog( gpointer *p, guint n, GtkWidget *pw ); 

typedef struct _toolbarwidget {

  GtkWidget *pwNew;        /* button for "New" */
  GtkWidget *pwOpen;       /* button for "Open" */
  GtkWidget *pwSave;     /* button for "Double" */
  GtkWidget *pwRedouble;   /* button for "Redouble" */
  GtkWidget *pwTake;       /* button for "Take" */
  GtkWidget *pwDrop;       /* button for "Drop" */
  GtkWidget *pwResign;       /* button for "Play" */
  GtkWidget *pwHint;      /* button for "Reset" */
  GtkWidget *pwReset;      /* button for "Reset" */
  GtkWidget *pwEdit;       /* button for "Edit" */
  GtkWidget *pwHideShowPanel; /* button hide/show panel */
  GtkWidget *pwButtonClockwise; /* button for clockwise */

} toolbarwidget;

/* Hack this for now to stop re-entering - should be fixed when menu switched to actions */
static int inCallback = FALSE;

static void ButtonClicked( GtkWidget *pw, char *sz ) {

    UserCommand( sz );
}


static void ButtonClickedYesNo( GtkWidget *pw, char *sz ) {

  if ( ms.fResigned ) {
    UserCommand ( ! strcmp ( sz, "yes" ) ? "accept" : "decline" );
    return;
  }

  if ( ms.fDoubled ) {
    UserCommand ( ! strcmp ( sz, "yes" ) ? "take" : "drop" );
    return;
  }

}


extern GtkWidget *
image_from_xpm_d ( char **xpm, GtkWidget *pw ) {

  GdkPixmap *ppm;
  GdkBitmap *mask;

  ppm = gdk_pixmap_colormap_create_from_xpm_d( NULL,
                 gtk_widget_get_colormap( pw ), &mask, NULL,
                                               xpm );
  if ( ! ppm )
    return NULL;
  
  return gtk_pixmap_new( ppm, mask );

}

static GtkWidget *
toggle_button_from_images( GtkWidget *pwImageOff,
                           GtkWidget *pwImageOn, char *sz ) {

  GtkWidget **aapw;
  GtkWidget *pwm = gtk_multiview_new();
  GtkWidget *pw = gtk_toggle_button_new();
  GtkWidget *pwvbox = gtk_vbox_new(FALSE, 0); 

  aapw = (GtkWidget **) g_malloc( 3 * sizeof ( GtkWidget *) );

  aapw[ 0 ] = pwImageOff;
  aapw[ 1 ] = pwImageOn;
  aapw[ 2 ] = pwm;

  gtk_container_add( GTK_CONTAINER( pwvbox ), pwm );

  gtk_container_add( GTK_CONTAINER( pwm ), pwImageOff );
  gtk_container_add( GTK_CONTAINER( pwm ), pwImageOn );
  gtk_container_add( GTK_CONTAINER( pwvbox ), gtk_label_new(sz));
  gtk_container_add( GTK_CONTAINER( pw ), pwvbox);

  
  g_object_set_data_full( G_OBJECT( pw ), "toggle_images", aapw, g_free );

  return pw;
  
}

extern void
ToolbarSetPlaying( GtkWidget *pwToolbar, const int f ) {

  toolbarwidget *ptw = g_object_get_data ( G_OBJECT ( pwToolbar ),
		  "toolbarwidget" );
  
  gtk_widget_set_sensitive( ptw->pwReset, f );

}


extern void
ToolbarSetClockwise( GtkWidget *pwToolbar, const int f ) {

  toolbarwidget *ptw = g_object_get_data ( G_OBJECT ( pwToolbar ),
		  "toolbarwidget" );
  
  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( ptw->pwButtonClockwise ), f );

}

extern void click_swapdirection(void)
{
	if (!inCallback)
	{
		toolbarwidget *ptw = g_object_get_data ( G_OBJECT ( pwToolbar ), "toolbarwidget" );
		gtk_button_clicked( GTK_BUTTON( ptw->pwButtonClockwise ));
	}
}

static void ToolbarToggleClockwise( GtkWidget *pw, toolbarwidget *ptw )
{
  GtkWidget **aapw = (GtkWidget **) g_object_get_data( G_OBJECT( pw ), "toggle_images" );
  int f = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pw ) );
  
  gtk_multiview_set_current( GTK_MULTIVIEW( aapw[ 2 ] ), aapw[ f ] );

  inCallback = TRUE;
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif, "/View/Play Clockwise" )), f);
  inCallback = FALSE;

  if ( fClockwise != f )
  {
    gchar *sz = g_strdup_printf( "set clockwise %s", f ? "on" : "off" );
    UserCommand( sz );
    g_free( sz );
  }
}

int editing = FALSE;

extern void click_edit(void)
{
	if (!inCallback)
	{
		toolbarwidget *ptw = g_object_get_data ( G_OBJECT ( pwToolbar ), "toolbarwidget" );
        gtk_button_clicked( GTK_BUTTON( ptw->pwEdit ));
	}
}

static void ToolbarToggleEdit(GtkWidget * pw, toolbarwidget * ptw)
{
	BoardData *pbd = BOARD(pwBoard)->board_data;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ptw->pwEdit))) {
		editing = TRUE;
		if (ms.gs == GAME_NONE)
			edit_new(nDefaultLength);
		/* Undo any partial move that may have been made when entering edit mode */
		GTKUndo();
	} else
		editing = FALSE;

	inCallback = TRUE;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget(pif, "/Edit/Edit Position" )), editing);
	inCallback = FALSE;

	board_edit(pbd);
}

extern int
ToolbarIsEditing( GtkWidget *pwToolbar ) {

  return editing;
}

extern toolbarcontrol
ToolbarUpdate ( GtkWidget *pwToolbar,
                const matchstate *pms,
                const DiceShown diceShown,
                const int fComputerTurn,
                const int fPlaying ) {

  toolbarwidget *ptw = g_object_get_data ( G_OBJECT ( pwToolbar ),
		  "toolbarwidget" );
  toolbarcontrol c;
  int fEdit = ToolbarIsEditing( pwToolbar );

  g_assert ( ptw );

  c = C_NONE;

  if ( diceShown == DICE_BELOW_BOARD )
    c = C_ROLLDOUBLE;

  if( pms->fDoubled )
    c = C_TAKEDROP;

  if( pms->fResigned )
    c = C_AGREEDECLINE;

  if( fComputerTurn )
    c = C_PLAY;

  if( fEdit || ! fPlaying )
    c = C_NONE;
#if 0
  gtk_widget_set_sensitive ( ptw->pwRoll, c == C_ROLLDOUBLE );
  gtk_widget_set_sensitive ( ptw->pwDouble, c == C_ROLLDOUBLE );

  gtk_widget_set_sensitive ( ptw->pwPlay, c == C_PLAY );
#endif

  gtk_widget_set_sensitive ( ptw->pwTake,
                             c == C_TAKEDROP || c == C_AGREEDECLINE );
  gtk_widget_set_sensitive ( ptw->pwDrop,
                             c == C_TAKEDROP || c == C_AGREEDECLINE );
  /* FIXME: max beavers etc */
  gtk_widget_set_sensitive ( ptw->pwRedouble,
                             c == C_TAKEDROP && ! pms->nMatchTo );

  gtk_widget_set_sensitive ( ptw->pwSave,  plGame != NULL );
  gtk_widget_set_sensitive ( ptw->pwResign, fPlaying  && !fEdit);
  gtk_widget_set_sensitive ( ptw->pwHint, fPlaying  && !fEdit);
  gtk_widget_set_sensitive ( ptw->pwEdit, TRUE );

  return c;

}


extern GtkWidget *
ToolbarNew ( void ) {

	/* FIXME in this function:
	 *
	 * For GTK 2.0 there should be _real_ stock icons, so the 
	 * the user cab change the theme
	 * 
	 */

  GtkWidget *vbox_toolbar;
  GtkWidget *pwToolbar, *pwvbox;

  toolbarwidget *ptw;
#if 0
#include "xpm/tb_roll.xpm"
#include "xpm/tb_double.xpm"
#include "xpm/tb_edit.xpm"
#endif
#include "xpm/beaver.xpm"
#include "xpm/tb_anticlockwise.xpm"
#include "xpm/tb_clockwise.xpm"
#include "xpm/resign.xpm"
#include "xpm/hint_alt.xpm"
#include "xpm/stock_new.xpm"
#include "xpm/stock_open.xpm"
#include "xpm/stock_save.xpm"
#include "xpm/stock_ok.xpm"
#include "xpm/stock_cancel.xpm"
#include "xpm/stock_undo.xpm"
#include "xpm/stock_edit.xpm"
    
  /* 
   * Create tool bar 
   */
  
  ptw = (toolbarwidget *) g_malloc ( sizeof ( toolbarwidget ) );

  vbox_toolbar = gtk_vbox_new (FALSE, 0);
  
  g_object_set_data_full ( G_OBJECT ( vbox_toolbar ), "toolbarwidget",
                             ptw, g_free );
  pwToolbar = gtk_toolbar_new ();
  gtk_toolbar_set_orientation ( GTK_TOOLBAR ( pwToolbar ),
                                GTK_ORIENTATION_HORIZONTAL );
  gtk_toolbar_set_style ( GTK_TOOLBAR ( pwToolbar ),
                          GTK_TOOLBAR_BOTH );
  gtk_box_pack_start( GTK_BOX( vbox_toolbar ), pwToolbar, 
                      FALSE, FALSE, 0 );

  gtk_toolbar_set_tooltips ( GTK_TOOLBAR ( pwToolbar ), TRUE );

  /* new button */
  
  gtk_toolbar_append_item ( GTK_TOOLBAR ( pwToolbar ),
			       _("New"),
                               _("Start new game, match, session or position"),
                               NULL,
			       image_from_xpm_d ( stock_new_xpm, pwToolbar),
                               G_CALLBACK( GTKNew ), NULL );
  /* Open button */
  gtk_toolbar_append_item ( GTK_TOOLBAR ( pwToolbar ),
			       _("Open"),
                              _("Open game, match, session or position"),
                               NULL,
			       image_from_xpm_d ( stock_open_xpm, pwToolbar),
                               G_CALLBACK( GTKOpen ), NULL );

#define TB_BUTTON_ADD(pointer,icon,label,cb,arg,tooltip,tooltip2) \
  pointer = gtk_button_new(); \
  pwvbox = gtk_vbox_new(FALSE, 0); \
  gtk_container_add(GTK_CONTAINER(pwvbox), \
		  image_from_xpm_d (icon, pwToolbar)); \
  gtk_container_add(GTK_CONTAINER(pwvbox), \
		  gtk_label_new(label)); \
  gtk_container_add(GTK_CONTAINER(pointer), pwvbox); \
  g_signal_connect(G_OBJECT(pointer), "clicked", \
		  G_CALLBACK( cb ), arg ); \
  gtk_toolbar_append_widget( GTK_TOOLBAR ( pwToolbar ), \
			    pointer, tooltip, tooltip2 ); \
  gtk_button_set_relief( GTK_BUTTON(pointer), \
		  GTK_RELIEF_NONE);

#define TB_TOGGLE_BUTTON_ADD(pointer,icon,label,cb,arg,tooltip,tooltip2) \
  pointer = gtk_toggle_button_new(); \
  pwvbox = gtk_vbox_new(FALSE, 0); \
  gtk_container_add(GTK_CONTAINER(pwvbox), \
		  image_from_xpm_d (icon, pwToolbar)); \
  gtk_container_add(GTK_CONTAINER(pwvbox), \
		  gtk_label_new(label)); \
  gtk_container_add(GTK_CONTAINER(pointer), pwvbox); \
  g_signal_connect(G_OBJECT(pointer), "toggled", \
		  G_CALLBACK( cb ), arg ); \
  gtk_toolbar_append_widget( GTK_TOOLBAR ( pwToolbar ), \
			    pointer, tooltip, tooltip2 ); \
  gtk_button_set_relief( GTK_BUTTON(pointer), \
		  GTK_RELIEF_NONE);
  
  /* Save button */
  TB_BUTTON_ADD(ptw->pwSave, stock_save_xpm, _("Save"), GTKSave,
		  NULL, 
                  _("Save match, session, game or position"), 
		  NULL) ;
  
  gtk_toolbar_append_space(GTK_TOOLBAR(pwToolbar));

  /* Take/accept button */
  TB_BUTTON_ADD(ptw->pwTake, stock_ok_xpm, _("Accept"), ButtonClickedYesNo,
		  "yes", 
                  _("Take the offered cube or accept the offered resignation"),
		  NULL) ;
  
  /* drop button */
  TB_BUTTON_ADD(ptw->pwDrop, stock_cancel_xpm, _("Decline"), ButtonClickedYesNo,
		  "no", 
                  _("Drop the offered cube or decline the offered resignation"),
		  NULL) ;

  /* Redouble button */
  TB_BUTTON_ADD(ptw->pwRedouble, beaver_xpm, _("Beaver"), ButtonClicked,
		  "redouble", 
                  _("Redouble immediately (beaver)"),
		  NULL) ;
  
  /* Resign button */
  TB_BUTTON_ADD(ptw->pwResign, resign_xpm, _("Resign"), GTKResign,
		  NULL, 
                  _("Resign the current game"),
		  NULL) ;

  /* play button */
  
  /* How often do you use the "play" button? I guess it's so seldom
   * you won't mind using the pulldown menues */
  
  gtk_toolbar_append_space(GTK_TOOLBAR(pwToolbar));
  
  /* reset button */
  TB_BUTTON_ADD(ptw->pwReset, stock_undo_xpm, _("Undo"), GTKUndo,
		  NULL, 
                  _("Undo moves"),
		  NULL) ;
  /* Hint button */
  TB_BUTTON_ADD(ptw->pwHint, hint_alt_xpm, _("Hint"), ButtonClicked,
		  "hint", 
                  _("Show the best moves or cube action"),
		  NULL) ;

  /* edit button */
  TB_TOGGLE_BUTTON_ADD(ptw->pwEdit, stock_edit_xpm, _("Edit"), 
		  ToolbarToggleEdit,
		  ptw, 
                  _("Edit position"),
		  NULL) ;

  /* direction of play */
  ptw->pwButtonClockwise = toggle_button_from_images ( 
		  image_from_xpm_d( tb_anticlockwise_xpm, pwToolbar),
		  image_from_xpm_d( tb_clockwise_xpm, pwToolbar),
                  _("Direction"));

  g_signal_connect(G_OBJECT(ptw->pwButtonClockwise), "toggled", 
		  G_CALLBACK( ToolbarToggleClockwise ), ptw );
  gtk_toolbar_append_widget( GTK_TOOLBAR ( pwToolbar ), 
			    ptw->pwButtonClockwise,
			    _("Reverse direction of play"), NULL ); 
  gtk_button_set_relief( GTK_BUTTON(ptw->pwButtonClockwise), 
		  GTK_RELIEF_NONE);

  gtk_toolbar_append_space(GTK_TOOLBAR(pwToolbar));
  
#undef TB_TOGGLE_BUTTON_ADD
#undef TB_BUTTON_ADD
  
  return vbox_toolbar;

}

