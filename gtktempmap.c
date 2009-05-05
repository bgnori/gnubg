
/*
 * gtktempmap.c
 *
 * by Joern Thyssen <jth@gnubg.org>, 2003
 *
 * Based on Sho Sengoku's Equity Temperature Map
 * http://www46.pair.com/sengoku/TempMap/English/TempMap.html
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
 * $Id: gtktempmap.c,v 1.42 2009/05/05 16:09:41 c_anthon Exp $
 */

#include "config.h"

#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "backgammon.h"
#include "eval.h"
#include "gtktempmap.h"
#include "gtkgame.h"
#include "drawboard.h"
#include "format.h"
#include "render.h"
#include "renderprefs.h"
#include "gtkboard.h"
#include "gtkgame.h"
#include "gtkwindows.h"

#define SIZE_DIE 4
#define SIZE_QUADRANT 52


typedef struct _tempmap {
  
  matchstate *pms;
  float aarEquity[ 6 ][ 6 ];
  float rAverage;

  GtkWidget *aapwDA[ 6 ][ 6 ];
  GtkWidget *aapwe[ 6 ][ 6 ];
  GtkWidget *pwAverage;

  GtkWidget *pweAverage;

  int aaanMove[ 6 ][ 6 ][ 8 ];

  gchar *szTitle;

} tempmap;


typedef struct _tempmapwidget {

  unsigned char *achDice[ 2 ];
  unsigned char *achPips[ 2 ];
  int fShowEquity;
  int fShowBestMove;
  int fInvert;
  GtkWidget *apwGauge[ 2 ];
  float rMin, rMax;

  tempmap *atm;
  int n;

  int nSizeDie;

} tempmapwidget;


static int
TempMapEquities( evalcontext *pec, matchstate *pms, 
                 float aarEquity[ 6 ][ 6 ], int aaanMove[ 6 ][ 6 ][ 8 ],
                 const gchar *szTitle, const float rFac ) {


  int i, j;
  float arOutput[ NUM_ROLLOUT_OUTPUTS ];
  TanBoard anBoard;
  int aaan[ 6 ][ 6 ][ 8 ];
  float aar[ 6 ][ 6 ];
  cubeinfo ci;
  cubeinfo cix;

  /* calculate equities */

  GetMatchStateCubeInfo( &cix, pms );

  if ( szTitle && *szTitle ) {
    gchar *sz = g_strdup_printf( _("Calculating equities for %s"), szTitle );
    ProgressStartValue ( sz, 21 );
    g_free( sz );
  }
  else
    ProgressStartValue ( _("Calculating equities" ), 21 );

  for ( i = 0; i < 6; ++i )
    for ( j = 0; j <= i; ++j ) {

      memcpy( &ci, &cix, sizeof ci );

      /* find best move */

      memcpy ( anBoard, pms->anBoard, sizeof ( anBoard ) );

      if ( FindBestMove ( aaan[ i ][ j ], i + 1, j + 1, anBoard, &ci, pec,
                          defaultFilters ) < 0 ) {
        ProgressEnd();
        return -1;
      }

      /* evaluate resulting position */

      SwapSides ( anBoard );
      ci.fMove = ! ci.fMove;

      if ( GeneralEvaluationE ( arOutput, (ConstTanBoard)anBoard, &ci, pec ) < 0 ) {
        ProgressEnd();
        return -1;
      }

      InvertEvaluationR( arOutput, &cix );

      if ( ! cix.nMatchTo && rFac != 1.0 )
        arOutput[ OUTPUT_CUBEFUL_EQUITY ] *= rFac;

      aar[ i ][ j ] = arOutput[ OUTPUT_CUBEFUL_EQUITY ];
      aar[ j ][ i ] = arOutput[ OUTPUT_CUBEFUL_EQUITY ];

      if ( i != j )
        memcpy( aaan[ j ][ i ], aaan[ i ][ j ], sizeof aaan[ 0 ][ 0 ] );
      
      ProgressValueAdd ( 1 );

    }



  ProgressEnd();

  memcpy( aarEquity, aar, sizeof aar );
  memcpy( aaanMove, aaan, sizeof aaan );

  return 0;

}


static int
CalcTempMapEquities( evalcontext *pec, tempmapwidget *ptmw ) {

  int i;

  for ( i = 0; i < ptmw->n; ++i )
    if ( TempMapEquities( pec, ptmw->atm[ i ].pms,
                          ptmw->atm[ i ].aarEquity, 
                          ptmw->atm[ i ].aaanMove,
                          ptmw->atm[ i ].szTitle,
                          (float)(ptmw->atm[ i ].pms->nCube / ptmw->atm[ 0 ].pms->nCube) ) < 0 ) 
      return -1;

  return 0;

}




static void
UpdateStyle( GtkWidget *pw, const float r ) {

  GtkStyle *ps = gtk_style_copy( pw->style );

  ps->bg[ GTK_STATE_NORMAL ].red = 0xFFFF;
  ps->bg[ GTK_STATE_NORMAL ].blue = 
  ps->bg[ GTK_STATE_NORMAL ].green = (guint16)(( 1.0f - r ) * 0xFFFF);

  gtk_widget_set_style( pw, ps ); 

}


static void
SetStyle( GtkWidget *pw, const float rEquity, 
          const float rMin, const float rMax, const int fInvert ) {

  float r = ( rEquity - rMin ) / ( rMax - rMin );
  UpdateStyle( pw, fInvert ? ( 1.0f - r ) : r );

}


static char *
GetEquityString( const float rEquity, cubeinfo *pci, const int fInvert ) {

  float r;

  if ( fInvert ) {
    /* invert equity */
    if ( pci->nMatchTo )
      r = 1.0f - rEquity;
    else
      r = -rEquity;
  }
  else
    r = rEquity;

  if ( fInvert ) {
    cubeinfo ci;
    memcpy( &ci, pci, sizeof ci );
    ci.fMove = ! ci.fMove;

    return OutputMWC( r, &ci, TRUE );
  }
  else
    return OutputMWC( r, pci, TRUE );

}


static void
UpdateTempMapEquities( tempmapwidget *ptmw ) {

  int i, j;
  float rMax, rMin, r;
  cubeinfo ci;
  int m;
  char szMove[ 100 ];

  /* calc. min, max and average */

  rMax = -10000;
  rMin = +10000;
  for ( m = 0; m < ptmw->n; ++m ) {
    ptmw->atm[ m ].rAverage = 0.0f;
    for ( i = 0; i < 6; ++i )
      for ( j = 0; j < 6; ++j ) {
        r = ptmw->atm[ m ].aarEquity[ i ][ j ];
        ptmw->atm[ m ].rAverage += r;
        if ( r > rMax )
          rMax = r;
        if ( r < rMin ) 
          rMin = r;
      }
    ptmw->atm[ m ].rAverage /= 36.0;
  }

  ptmw->rMax = rMax;
  ptmw->rMin = rMin;

  /* update styles */

  GetMatchStateCubeInfo ( &ci, ptmw->atm[ 0 ].pms );


  for ( m = 0; m < ptmw->n; ++m ) {
    for ( i = 0; i < 6; ++i )
      for ( j = 0; j < 6; ++j ) {

        gchar *sz = 
          g_strdup_printf( "%s [%s]", 
                           GetEquityString( ptmw->atm[ m ].aarEquity[ i ][ j ],
                                            &ci, ptmw->fInvert ), 
                           FormatMove( szMove, (ConstTanBoard)ptmw->atm[ m ].pms->anBoard, 
                                       ptmw->atm[ m ].aaanMove[ i ][ j ] ) );
                           

        SetStyle( ptmw->atm[ m ].aapwDA[ i ][ j ],
                  ptmw->atm[ m ].aarEquity[ i ][ j ], rMin, rMax, 
                  ptmw->fInvert );

        gtk_widget_set_tooltip_text(ptmw->atm[ m ].aapwe[ i ][ j ], sz);
        g_free( sz );
        gtk_widget_queue_draw( ptmw->atm[ m ].aapwDA[ i ][ j ] ); 

      }

    SetStyle( ptmw->atm[ m ].pwAverage,
              ptmw->atm[ m ].rAverage, rMin, rMax, ptmw->fInvert );

    gtk_widget_set_tooltip_text(ptmw->atm[ m ].pweAverage,
                          GetEquityString( ptmw->atm[ m ].rAverage, 
                                           &ci, ptmw->fInvert ));
    gtk_widget_queue_draw( ptmw->atm[ m ].pwAverage );

  }

  /* update labels on gauge */

  gtk_label_set_text( GTK_LABEL( ptmw->apwGauge[ ptmw->fInvert ] ),
                      GetEquityString( rMin, &ci, ptmw->fInvert ) );
  gtk_label_set_text( GTK_LABEL( ptmw->apwGauge[ ! ptmw->fInvert ] ),
                      GetEquityString( rMax, &ci, ptmw->fInvert ) );
} 

static void ExposeQuadrant(GtkWidget * pw, GdkEventExpose *UNUSED(pev), tempmapwidget *ptmw)
{
	int *pi = (int *) g_object_get_data(G_OBJECT(pw), "user_data");
	int i = 0;
	int j = 0;
	int m = 0;
	float r = 0.0f;
	cubeinfo ci;
	PangoLayout *layout;
	PangoFontDescription *description;
	float y;
	GString *str;
	char *pch, *tmp;

	gtk_paint_box(pw->style, pw->window, GTK_STATE_NORMAL, GTK_SHADOW_IN, NULL, NULL, NULL,
		      0, 0, pw->allocation.width, pw->allocation.height);
	if (pi == NULL)
		return;

	if (*pi >= 0) {
		i = (*pi % 100) / 6;
		j = (*pi % 100) % 6;
		m = *pi / 100;
	} else {
		m = -(*pi + 1);
		j = -1;
	}

	str = g_string_new("");

	if (ptmw->fShowEquity) {
		if (j >= 0)
			r = ptmw->atm[m].aarEquity[i][j];
		else if (j == -1)
			r = ptmw->atm[m].rAverage;
		GetMatchStateCubeInfo(&ci, ptmw->atm[m].pms);
		tmp = GetEquityString( r, &ci, ptmw->fInvert );
		while (*tmp == ' ')
			tmp++;
		g_string_append(str, tmp);
	}


	/* move */

	if (j >= 0 && ptmw->fShowBestMove) {
		char szMove[100];
		FormatMovePlain(szMove, ptmw->atm[m].pms->anBoard, ptmw->atm[m].aaanMove[i][j]);
		if (ptmw->fShowEquity)
			g_string_append_printf(str, " %s", szMove);
		else
			g_string_append(str, szMove);
	}

	if (str->len == 0)
	{
		g_string_free(str, TRUE);
		return;
	}

	pch = str->str;
	if (ptmw->fShowEquity && j >= 0 && ptmw->fShowBestMove)
		y = 2;
	else if (ptmw->fShowEquity)
		y = (pw->allocation.height - 4) / 2.0f;
	else
		y = 2 +(pw->allocation.height - 4) / 10.0f;

	description = pango_font_description_from_string("sans");
	pango_font_description_set_size(description, (pw->allocation.width / 8.0f) * PANGO_SCALE);
	layout = gtk_widget_create_pango_layout(pw, NULL);
	pango_layout_set_font_description(layout, description);
	do
	{
		tmp = strchr(pch, ' ');
		if (tmp)
			*tmp = 0;
		pango_layout_set_text(layout, pch, -1);
		gtk_paint_layout(pw->style, pw->window, GTK_STATE_NORMAL, TRUE, NULL, pw, NULL, 2, (int) y, layout);
		pch = tmp + 1;
		y += (pw->allocation.height - 4) / 5.0f;
	} while (tmp);

	g_object_unref(layout);
	g_string_free(str, TRUE);
}

static void
ExposeDie( GtkWidget *pw, GdkEventExpose *pev,
           tempmapwidget *ptmw ) {

  int *pi = (int *) g_object_get_data( G_OBJECT( pw ), "user_data" );
  GdkGC *gc = ( ( BoardData *) ( BOARD( pwBoard ) )->board_data )->gc_copy;
  int x, y;
  int nSizeDie;

  nSizeDie = ( pw->allocation.width - 4 ) / 7;
  if ( nSizeDie > ( ( pw->allocation.height - 4 ) / 7 ) )
    nSizeDie = ( pw->allocation.height - 4 )/ 7;

  if ( ptmw->nSizeDie != nSizeDie ) {

    int i;
    renderdata rd;

    /* render die */

    CopyAppearance(&rd);
    rd.nSize = ptmw->nSizeDie = nSizeDie;
#if USE_BOARD3D
    Copy3dDiceColour(&rd);
#endif
    
    for (  i = 0; i < 2; ++i ) {
      g_free( ptmw->achDice[ i ] );
      g_free( ptmw->achPips[ i ] );

      ptmw->achDice[ i ] = (unsigned char*)g_malloc ( nSizeDie * nSizeDie * 7 * 7 * 4 );
      ptmw->achPips[ i ] = (unsigned char*)g_malloc ( nSizeDie * nSizeDie * 3 );
    }

    RenderDice( &rd, ptmw->achDice[ 0 ], ptmw->achDice[ 1 ], nSizeDie * 7 * 4 );
    RenderPips( &rd, ptmw->achPips[ 0 ], ptmw->achPips[ 1 ], nSizeDie * 3 );

  }

  x = ( pw->allocation.width - ptmw->nSizeDie * 7 ) / 2;
  y = ( pw->allocation.height - ptmw->nSizeDie * 7 ) / 2;

  gdk_window_clear_area( pw->window, pev->area.x, pev->area.y,
			 pev->area.width, pev->area.height);
  DrawDie( pw->window, ptmw->achDice, ptmw->achPips, ptmw->nSizeDie,
           gc, x, y, ptmw->atm[ 0 ].pms->fMove, *pi + 1 );

}


static void
TempMapPlyToggled( GtkWidget *pw, tempmapwidget *ptmw ) {

  int *pi = (int *) g_object_get_data( G_OBJECT( pw ), "user_data" );
#if defined(REDUCTION_CODE)
  evalcontext ec = { TRUE, 0, 0, TRUE, 0.0 };
#else
  evalcontext ec = { TRUE, 0, FALSE, TRUE, 0.0 };
#endif
  
  if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pw ) ) ) {

    /* recalculate equities */

    ec.nPlies = *pi;

    if ( CalcTempMapEquities( &ec, ptmw ) )
      return;
    
    UpdateTempMapEquities( ptmw );

  }

}


static void
ShowEquityToggled( GtkWidget *pw, tempmapwidget *ptmw ) {

  int f = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pw ) );

  if ( f != ptmw->fShowEquity ) {
    ptmw->fShowEquity = f;
    UpdateTempMapEquities( ptmw );
  }

}


static void
ShowBestMoveToggled( GtkWidget *pw, tempmapwidget *ptmw ) {

  int f = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pw ) );

  if ( f != ptmw->fShowBestMove ) {
    ptmw->fShowBestMove = f;
    UpdateTempMapEquities( ptmw );
  }
}

static void DestroyDialog(gpointer p, GObject *UNUSED(obj))
{
  tempmapwidget *ptmw = (tempmapwidget *) p;
  int i;

  /* garbage collect */

  g_free( ptmw->achDice[ 0 ] );
  g_free( ptmw->achDice[ 1 ] );
  g_free( ptmw->achPips[ 0 ] );
  g_free( ptmw->achPips[ 1 ] );

  for ( i = 0; i < ptmw->n; ++i ) {
    g_free( ptmw->atm[ i ].pms );
    g_free( ptmw->atm[ i ].szTitle );
  }

  g_free( ptmw->atm );

  g_free( ptmw );

}

extern void
GTKShowTempMap( const matchstate ams[], const int n,
                const gchar *aszTitle[], const int fInvert ) {

#if defined(REDUCTION_CODE)
  evalcontext ec = { TRUE, 0, 0, TRUE, 0.0 };
#else
  evalcontext ec = { TRUE, 0, FALSE, TRUE, 0.0 };
#endif

  tempmapwidget *ptmw;
  int *pi;
  int i, j;

  GtkWidget *pwDialog;
  GtkWidget *pwTable = NULL;
  GtkWidget *pwv;
  GtkWidget *pw;
  GtkWidget *pwh;
  GtkWidget *pwx = NULL;
  GtkWidget *pwOuterTable;

  int k, l, km, lm, m;

  /* dialog */

  pwDialog = GTKCreateDialog( _("Sho Sengoku Temperature Map - Distribution of rolls"),
	  DT_INFO, NULL, DIALOG_FLAG_MODAL, NULL, NULL );

  ptmw = (tempmapwidget *) g_malloc( sizeof ( tempmapwidget ) );
  ptmw->fShowBestMove = FALSE;
  ptmw->fShowEquity = FALSE;
  ptmw->fInvert = fInvert;
  ptmw->n = n;
  ptmw->nSizeDie = -1;
  ptmw->achDice[ 0 ] = ptmw->achDice[ 1 ] = NULL;
  ptmw->achPips[ 0 ] = ptmw->achPips[ 1 ] = NULL;

  ptmw->atm = (tempmap *) g_malloc( n * sizeof ( tempmap ) );
  for ( i = 0; i < n; ++i ) {
    ptmw->atm[ i ].pms = (matchstate *) g_malloc( sizeof ( matchstate ) );
    memcpy( ptmw->atm[ i ].pms, &ams[ i ], sizeof ( matchstate ) );
  }

  /* vbox to hold tree widget and buttons */

  pwv = gtk_vbox_new ( FALSE, 8 );
  gtk_container_set_border_width ( GTK_CONTAINER ( pwv ), 8);
  gtk_container_add ( GTK_CONTAINER (DialogArea( pwDialog, DA_MAIN ) ), pwv );

  /* calculate number of rows and columns */

  for ( lm = 1; /**/; ++lm )
    if ( lm * lm >= n )
      break;

  for ( km = 1; km * lm < n; ++km )
    ;

  pwOuterTable = gtk_table_new( km, lm, TRUE );
  gtk_box_pack_start( GTK_BOX( pwv ), pwOuterTable, TRUE, TRUE, 0 );

  for ( k = m = 0; k < km; ++k )
    for ( l = 0; l < lm && m < n; ++l, ++m ) {

      tempmap *ptm = &ptmw->atm[ m ];

      ptm->szTitle = ( aszTitle &&  aszTitle[ m ] && *aszTitle[ m ] ) ?
        g_strdup( aszTitle[ m ] ) : NULL;

      pw = gtk_frame_new( ptm->szTitle );

      gtk_table_attach_defaults( GTK_TABLE( pwOuterTable ),
                                 pw, l, l + 1, k, k + 1 );

      gtk_container_add( GTK_CONTAINER( pw ), 
                         pwTable = gtk_table_new( 7, 7, TRUE ) );

      /* drawing areas */

      for ( i = 0; i < 6; ++i ) {
        for ( j = 0; j < 6; ++j ) {
          
          ptm->aapwDA[ i ][ j ] = gtk_drawing_area_new();
          ptm->aapwe[ i ][ j ] = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(ptm->aapwe[i][j]), FALSE);
          
          gtk_container_add( GTK_CONTAINER( ptm->aapwe[ i ][ j ] ),
                             ptm->aapwDA[ i ][ j ] );
          
          gtk_widget_set_size_request( ptm->aapwDA[ i ][ j ] ,
                                 SIZE_QUADRANT, SIZE_QUADRANT );
          
          gtk_table_attach_defaults( GTK_TABLE( pwTable ), 
                                     ptm->aapwe[ i ][ j ],
                                     i + 1, i + 2,
                                     j + 1, j + 2 );
          
          pi = (int *) g_malloc( sizeof ( int ) );
          *pi = i * 6 + j + m * 100;
          
          g_object_set_data_full( G_OBJECT( ptm->aapwDA[ i ][ j ] ),
                                    "user_data", pi, g_free );
          
          g_signal_connect( G_OBJECT( ptm->aapwDA[ i ][ j ] ),
                              "expose_event",
                              G_CALLBACK( ExposeQuadrant ), ptmw );

        }

        /* die */

        pw = gtk_drawing_area_new();
        gtk_widget_set_size_request(  pw, SIZE_QUADRANT, SIZE_QUADRANT );
        
        gtk_table_attach_defaults( GTK_TABLE( pwTable ),
                                   pw, 0, 1, i + 1, i + 2 );

        pi = (int *) g_malloc( sizeof ( int ) );
        *pi = i;
        
        g_object_set_data_full( G_OBJECT( pw ),
                                  "user_data", pi, g_free );
        
        g_signal_connect( G_OBJECT( pw ),
                            "expose_event",
                            G_CALLBACK( ExposeDie ),
                            ptmw );
        /* die */
    
        pw = gtk_drawing_area_new();
        gtk_widget_set_size_request( pw, SIZE_QUADRANT, SIZE_QUADRANT );
    
        gtk_table_attach_defaults( GTK_TABLE( pwTable ),
                                   pw, i + 1, i + 2, 0, 1 );
        
        pi = (int *) g_malloc( sizeof ( int ) );
        *pi = i;
        
        g_object_set_data_full( G_OBJECT( pw ),
                                  "user_data", pi, g_free );
        
        g_signal_connect( G_OBJECT( pw ),
                            "expose_event",
                            G_CALLBACK( ExposeDie ),
                            ptmw );
    
      }

      /* drawing area for average */
      
      ptm->pwAverage = gtk_drawing_area_new();
      ptm->pweAverage = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(ptm->pweAverage), FALSE);

      gtk_container_add( GTK_CONTAINER( ptm->pweAverage ), ptm->pwAverage );

      gtk_widget_set_size_request( ptm->pwAverage, SIZE_QUADRANT, SIZE_QUADRANT );
      
      gtk_table_attach_defaults( GTK_TABLE( pwTable ), 
                                 ptm->pweAverage,
                                 0, 1, 0, 1 );
      
      pi = (int *) g_malloc( sizeof ( int ) );
      *pi = -m - 1;
      
      g_object_set_data_full( G_OBJECT( ptm->pwAverage ),
                                "user_data", pi, g_free );
      
      g_signal_connect( G_OBJECT( ptm->pwAverage ),
                          "expose_event",
                          G_CALLBACK( ExposeQuadrant ),
                          ptmw );

     }

  /* separator */

  gtk_box_pack_start( GTK_BOX( pwv ), gtk_hseparator_new(),
                      FALSE, FALSE, 0 );

  /* gauge */

  pwTable = gtk_table_new( 2, 16, FALSE ); 
  gtk_box_pack_start( GTK_BOX( pwv ), pwTable, FALSE, FALSE, 0 );

  for ( i = 0; i < 16; ++i ) {

    pw = gtk_drawing_area_new();
    gtk_widget_set_size_request( pw, 15, 20 );
    
    gtk_table_attach_defaults( GTK_TABLE( pwTable ), 
                               pw,
                               i, i + 1,
                               1, 2 );
    
    g_object_set_data( G_OBJECT( pw ), "user_data", NULL );
    
    g_signal_connect( G_OBJECT( pw ),
                        "expose_event",
                        G_CALLBACK( ExposeQuadrant ), NULL );
    
    UpdateStyle( pw, 1.0f * i / 15.0f );
    
    
  }

  for ( i = 0; i < 2; ++i ) {
    ptmw->apwGauge[ i ] = gtk_label_new( "" );
    gtk_table_attach_defaults( GTK_TABLE( pwTable ), ptmw->apwGauge[ i ], 
                               15 * i, 15 * i + 1, 0, 1 );
  }

  /* separator */

  gtk_box_pack_start( GTK_BOX( pwv ), gtk_hseparator_new(), FALSE, FALSE, 0 );


  /* buttons */

  pwh = gtk_hbox_new( FALSE, 4 );
  gtk_box_pack_start( GTK_BOX( pwv ), pwh, FALSE, FALSE, 0 );
  
  for ( i = 0; i < 4; ++i ) {

    gchar *sz = g_strdup_printf( _("%d ply"), i );
    if ( ! i )
      pw = pwx = gtk_radio_button_new_with_label( NULL, sz );
    else
      pw = 
        gtk_radio_button_new_with_label_from_widget( GTK_RADIO_BUTTON( pwx ),
                                                     sz );
    g_free(sz);
      
    gtk_box_pack_start( GTK_BOX( pwh ), pw, FALSE, FALSE, 0 );

    pi = (int *) g_malloc( sizeof ( int ) );
    *pi = i;
    
    g_object_set_data_full( G_OBJECT( pw ),
                              "user_data", pi, g_free );
    
    g_signal_connect( G_OBJECT( pw ), "toggled", 
                        G_CALLBACK( TempMapPlyToggled ), ptmw );
                        
  }

  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( pwx ), TRUE );


  /* show-buttons */

  pwh = gtk_hbox_new( FALSE, 4 );
  gtk_box_pack_start( GTK_BOX( pwv ), pwh, FALSE, FALSE, 0 );
  
  pw = gtk_check_button_new_with_label( _("Show equities") );
  gtk_box_pack_end( GTK_BOX( pwh ), pw, FALSE, FALSE, 0 );
  
  g_signal_connect( G_OBJECT( pw ), "toggled",
                      G_CALLBACK( ShowEquityToggled ), ptmw );


  pw = gtk_check_button_new_with_label( _("Show best move") );
  gtk_box_pack_end( GTK_BOX( pwh ), pw, FALSE, FALSE, 0 );
  
  g_signal_connect( G_OBJECT( pw ), "toggled",
                      G_CALLBACK( ShowBestMoveToggled ), ptmw );


  /* update */

  CalcTempMapEquities( &ec, ptmw );
  UpdateTempMapEquities( ptmw );

  /* modality */

  gtk_window_set_default_size( GTK_WINDOW( pwDialog ), 400, 500 ); 
  g_object_weak_ref( G_OBJECT( pwDialog ), DestroyDialog, ptmw );

  GTKRunDialog(pwDialog);
}
