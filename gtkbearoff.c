/*
 * gtkbearoff.c
 *
 * by Joern Thyssen <jth@gnubg.org>, 2003
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
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
 * $Id: gtkbearoff.c,v 1.20 2007/03/17 15:42:48 Superfly_Jon Exp $
 */

#include <config.h>

#include <gtk/gtk.h>

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <stdlib.h>

#include "backgammon.h"
#include "gtkbearoff.h"
#include "gtkgame.h"
#include "gtktoolbar.h"
#include "bearoff.h"
#include <glib/gi18n.h>
#include "gtkwindows.h"

typedef struct _bearoffwidget {

  GtkWidget *pw;
  GtkWidget *pwText;
  GtkWidget *apwWho[ 2 ];
  GtkWidget *apwRoll[ 2 ];
  GtkWidget *apwDice[ 2 ];
  matchstate ms;
  bearoffcontext *pbc;

} bearoffwidget;


typedef struct _sconyerswidget {

  bearoffwidget *pbw15x15;
  bearoffwidget *pbwTS;
  bearoffwidget *pbw2;
  matchstate ms;

} sconyerswidget;

static void
DestroyDialog( gpointer p ) {

  sconyerswidget *psw = (sconyerswidget *) p;

  g_free( psw );

}


static void
ToggleWho( GtkWidget *pw, bearoffwidget *pbw ) {

  int f = 
    gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pbw->apwWho[ 0 ] ) );

  if ( f != pbw->ms.fMove ) {
    pbw->ms.fMove = f;
    SwapSides( pbw->ms.anBoard );
  }

}


static void
BearoffUpdated( GtkWidget *pw, bearoffwidget *pbw ) {

  int i;
  int f;
  gchar *pch;
  GtkTextBuffer *buffer;
  PangoFontDescription *font_desc;

  /* read values */

  pbw->ms.fMove = 
    gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pbw->apwWho[ 0 ] ) );

  f = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pbw->apwRoll[ 0 ] ) );

  if ( f ) 
    /* player on roll */
    pbw->ms.anDice[ 0 ] = pbw->ms.anDice[ 1 ] = 0;
  else
    /* player has rolled */
    for ( i = 0; i < 2; ++i )
      pbw->ms.anDice[ i ] = 1 +
        gtk_option_menu_get_history( GTK_OPTION_MENU( pbw->apwDice[ i ] ) );

  /* get text */

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pbw->pwText));

  pch = (char*)malloc( 2000 );
  strcpy( pch, "" );
  ShowBearoff( pch, &pbw->ms, pbw->pbc );

  font_desc = pango_font_description_from_string ("Monospace");
  gtk_widget_modify_font (pbw->pwText, font_desc);
  pango_font_description_free (font_desc);
  gtk_text_buffer_set_text(buffer, pch, -1);

  free( pch );

}


static void
BearoffSet( bearoffwidget *pbw ) {

  int i;

  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( pbw->apwWho[ pbw->ms.fMove ] ), TRUE );

  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( pbw->apwRoll[ pbw->ms.anDice[ 0 ] > 0 ] ), TRUE );
  
  if ( pbw->ms.anDice[ 0 ] > 0 ) {
    for( i = 0; i < 2; ++i ) 
      gtk_option_menu_set_history( GTK_OPTION_MENU( pbw->apwDice[ i ] ),
                                   pbw->ms.anDice[ i ] - 1 );
  }
  
  /* force update */

  BearoffUpdated( NULL, pbw );

}


static bearoffwidget *
CreateBearoff( matchstate *pms, bearoffcontext *pbc ) {
#include "xpm/dice.xpm"

  GtkWidget *pwv;
  GtkWidget *pwh;
  GtkWidget *pw;
  GtkWidget *pwItem;
  GtkWidget *pwImage;
  bearoffwidget *pbw;
  int i, j;
  char **aaXpm[6];
  
  if ( ( pbw = (bearoffwidget *) g_malloc( sizeof ( bearoffwidget ) ) ) == NULL )
    return NULL;

  memcpy( &pbw->ms, pms, sizeof ( matchstate ) );
  pbw->pbc = pbc;

  pwv = gtk_vbox_new( FALSE, 0 );
  pbw->pw = pwv;
  gtk_object_set_data_full( GTK_OBJECT( pwv ), "user_data", pbw, g_free );
  
  /* 
   * radio buttons 
   */

  /* player */

  pbw->apwWho[ 0 ] = gtk_radio_button_new_with_label( NULL, ap[ 0 ].szName );
  pbw->apwWho[ 1 ] = 
    gtk_radio_button_new_with_label_from_widget( GTK_RADIO_BUTTON( pbw->apwWho[ 0 ] ), 
                                                 ap[ 1 ].szName );
  pwh = gtk_hbox_new( FALSE, 4 );
  gtk_box_pack_start( GTK_BOX( pwv ), pwh, FALSE, FALSE, 0 );

  gtk_box_pack_start( GTK_BOX( pwh ), 
                      gtk_label_new( _("Player on roll:") ), 
                      FALSE, FALSE, 4 );

  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( pbw->apwWho[ pbw->ms.fMove ] ), TRUE );

  for ( i = 0; i < 2; ++i ) {
    gtk_box_pack_start( GTK_BOX( pwh ), pbw->apwWho[ i ], FALSE, FALSE, 0 );
    gtk_signal_connect( GTK_OBJECT( pbw->apwWho[ i ] ), "toggled",
                        GTK_SIGNAL_FUNC( ToggleWho ), pbw );
    gtk_signal_connect( GTK_OBJECT( pbw->apwWho[ i ] ), "toggled",
                        GTK_SIGNAL_FUNC( BearoffUpdated ), pbw );
  }

  /* on roll or rolled */

  pbw->apwRoll[ 0 ] = gtk_radio_button_new_with_label( NULL, _("On roll") );
  pbw->apwRoll[ 1 ] = 
    gtk_radio_button_new_with_label_from_widget( GTK_RADIO_BUTTON( pbw->apwRoll[ 0 ] ),
                                                 _("Rolled") );
  pwh = gtk_hbox_new( FALSE, 4 );
  gtk_box_pack_start( GTK_BOX( pwv ), pwh, FALSE, FALSE, 0 );

  for ( i = 0; i < 2; ++i ) {
    gtk_box_pack_start( GTK_BOX( pwh ), pbw->apwRoll[ i ], FALSE, FALSE, 0 );
    gtk_signal_connect( GTK_OBJECT( pbw->apwRoll[ i ] ), "toggled",
                        GTK_SIGNAL_FUNC( BearoffUpdated ), pbw );
  }

  aaXpm[0] = dice1_xpm;
  aaXpm[1] = dice2_xpm;
  aaXpm[2] = dice3_xpm;
  aaXpm[3] = dice4_xpm;
  aaXpm[4] = dice5_xpm;
  aaXpm[5] = dice6_xpm;

  for ( i = 0; i < 2; ++i ) {
    char sz[ 2 ];
    int die = pbw->ms.anDice[ i ];


    
    pw = gtk_menu_new();
    
    for ( j = 0; j < 6; ++j ) {
      sprintf( sz, "%1d", j + 1 );
      pwItem = gtk_menu_item_new();
      pwImage = image_from_xpm_d(aaXpm[j], pw);
      gtk_container_add(GTK_CONTAINER(pwItem), pwImage);
      gtk_menu_append( GTK_MENU( pw ), pwItem );
      gtk_signal_connect( GTK_OBJECT( pwItem ), "activate",
                          GTK_SIGNAL_FUNC( BearoffUpdated ), pbw );
    }

    pbw->apwDice[ i ] = gtk_option_menu_new();
    gtk_widget_set_usize(pbw->apwDice[ i ], 70, 50 );

    gtk_option_menu_set_menu( GTK_OPTION_MENU( pbw->apwDice[ i ] ), pw );

    if ( die > 0 ) 
      gtk_option_menu_set_history( GTK_OPTION_MENU( pbw->apwDice[ i ] ),
                                   die - 1 );

    gtk_box_pack_start( GTK_BOX( pwh ), pbw->apwDice[ i ], FALSE, FALSE, 0 );

  }


  /* text widget for output */

  pbw->pwText = gtk_text_view_new();
  gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW( pbw->pwText ), GTK_WRAP_NONE );
  gtk_box_pack_end( GTK_BOX( pwv ), pbw->pwText, TRUE, TRUE, 0 );
  
  return pbw;

}


extern void
GTKShowBearoff( const matchstate *pms )
{
  GtkWidget *pwDialog;
  GtkWidget *pwNotebook;
  GtkWidget *pwv;
  sconyerswidget *psw;

  pwDialog = GTKCreateDialog( _("Bearoff Databases"), DT_INFO, NULL, DIALOG_FLAG_MODAL, NULL, NULL );

  pwv = gtk_vbox_new ( FALSE, 8 );
  gtk_container_set_border_width ( GTK_CONTAINER ( pwv ), 8);
  gtk_container_add ( GTK_CONTAINER (DialogArea( pwDialog, DA_MAIN ) ), pwv );

  pwNotebook = gtk_notebook_new();
  gtk_box_pack_start( GTK_BOX( pwv ), pwNotebook, TRUE, TRUE, 0 );

  psw = (sconyerswidget *) g_malloc( sizeof ( sconyerswidget ) );
  memset( psw, 0, sizeof ( sconyerswidget ) );

  memcpy( &psw->ms, pms, sizeof ( matchstate ) );
  
  /* add page for 15x15 bearoff database */
  
  if ( pbc15x15_dvd ) {
    psw->pbw15x15 = CreateBearoff( &psw->ms, pbc15x15_dvd );
      /* psw->pbw15x15 = CreateBearoff( &psw->ms, pbc2 ); */
    BearoffSet( psw->pbw15x15 );
    gtk_notebook_append_page( GTK_NOTEBOOK( pwNotebook ), 
                              psw->pbw15x15->pw, 
                              gtk_label_new( _("Sconyers' Bearoff (15x15)") ) );
  }

  if ( pbcTS ) {
    psw->pbwTS = CreateBearoff( &psw->ms, pbcTS );
    BearoffSet( psw->pbwTS );
    gtk_notebook_append_page( GTK_NOTEBOOK( pwNotebook ), 
                              psw->pbwTS->pw, 
                              gtk_label_new( _("Two-sided (disk)") ) );
  }
  else if ( pbc2 ) {
    psw->pbw2 = CreateBearoff( &psw->ms, pbc2 );
    BearoffSet( psw->pbw2 );
    gtk_notebook_append_page( GTK_NOTEBOOK( pwNotebook ), 
                              psw->pbw2->pw, 
                              gtk_label_new( _("Two-sided (memory)") ) );
  }
       
  /* show dialog */
  
  gtk_window_set_default_size( GTK_WINDOW( pwDialog ), 500, 500 ); 
  gtk_object_weakref( GTK_OBJECT( pwDialog ), DestroyDialog, psw );
  gtk_widget_show_all( pwDialog );
  gtk_main();
}



extern void
GTKShowEPC( int anBoard[ 2 ][ 25 ] )
{
  GtkWidget *pwDialog;
  GtkWidget *pwText;
  gchar *pch;
  GtkTextBuffer *buffer;
  PangoFontDescription *font_desc;

  pwDialog = GTKCreateDialog( _("Effective Pip Count"), DT_INFO, NULL, DIALOG_FLAG_MODAL, NULL, NULL );


  pwText = gtk_text_view_new();
  gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW(pwText), GTK_WRAP_NONE );
  gtk_text_view_set_editable (GTK_TEXT_VIEW(pwText), FALSE);
  gtk_container_add ( GTK_CONTAINER (DialogArea( pwDialog, DA_MAIN ) ), 
                      pwText );

  /* content */

  pch = ShowEPC( anBoard );

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pwText));

  font_desc = pango_font_description_from_string ("Monospace");
  gtk_widget_modify_font (pwText, font_desc);
  pango_font_description_free (font_desc);
  gtk_text_buffer_set_text(buffer, pch, -1);

  g_free( pch );

  /* show dialog */

  gtk_window_set_default_size( GTK_WINDOW( pwDialog ), 500, 500 ); 
  gtk_widget_show_all( pwDialog );
    
  GTKDisallowStdin();
  gtk_main();
  GTKAllowStdin();
}
