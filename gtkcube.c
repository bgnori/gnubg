/*
 * gtkcube.c
 *
 * by Joern Thyssen <jthyssen@dk.ibm.com>, 2002
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
 * $Id: gtkcube.c,v 1.10 2002/08/07 18:38:20 thyssen Exp $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_ALLOCA_H
#include <alloca.h>
#endif
#define GTK_ENABLE_BROKEN /* for GtkText */
#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "backgammon.h"
#include "eval.h"
#include "export.h"
#include "rollout.h"
#include "gtkgame.h"
#include "gtkcube.h"
#include "i18n.h"



typedef struct _cubehintdata {
  GtkWidget *pwFrame;   /* the table */
  GtkWidget *pw;        /* the box */
  GtkWidget *pwTools;   /* the tools */
  float (*aarOutput)[ NUM_ROLLOUT_OUTPUTS ];
  float (*aarStdDev)[ NUM_ROLLOUT_OUTPUTS ];
  float *arDouble;
  evalsetup *pes;
  movetype mt;
  matchstate ms;
} cubehintdata;



static GtkWidget *TakeAnalysis( const movetype mt, 
                                float aarOutput[][ NUM_ROLLOUT_OUTPUTS ],
                                float aarStdDev[][ NUM_ROLLOUT_OUTPUTS ],
				const evalsetup *pes,
                                matchstate *pms ) {

    cubeinfo ci;

    GtkWidget *pw;
    GtkWidget *pwTable;
    GtkWidget *pwFrame;

    int iRow;
    int i;
    cubedecision cd;

    int ai[ 2 ];
    const char *aszCube[] = {
      NULL, NULL, 
      N_("Take"), 
      N_("Pass") };

    float arDouble[ 4 ];
    gchar *sz;


    if( pes->et == EVAL_NONE )
	return NULL;

    GetMatchStateCubeInfo( &ci, pms );

    cd = FindCubeDecision ( arDouble, aarOutput, &ci );
    
    /* header */

    pwFrame = gtk_frame_new ( _("Take analysis") );
    gtk_container_set_border_width ( GTK_CONTAINER ( pwFrame ), 8 );

    pwTable = gtk_table_new ( 5, 4, FALSE );
    gtk_container_add ( GTK_CONTAINER ( pwFrame ), pwTable );

    /* if EVAL_EVAL include cubeless equity and winning percentages */

    iRow = 0;

    if ( pes->et == EVAL_EVAL ) 
      sz = g_strdup_printf ( ( ! ci.nMatchTo || ( ci.nMatchTo && ! fOutputMWC ) ) ?
                             _("Cubeless %d-ply equity: %s") :
                             _("Cubeless %d-ply MWC: %s"), 
                             pes->ec.nPlies,
                             OutputEquity ( aarOutput[ 0 ][ OUTPUT_EQUITY ], &ci, TRUE ) );
    else 
      sz = g_strdup_printf ( ( ! ci.nMatchTo || ( ci.nMatchTo && ! fOutputMWC ) ) ?
                             _("Cubeless rollout equity: %s") :
                             _("Cubeless rollout MWC: %s"), 
                             OutputEquity ( aarOutput[ 0 ][ OUTPUT_EQUITY ], &ci, TRUE ) );

    pw = gtk_label_new ( sz );
    gtk_misc_set_alignment( GTK_MISC( pw ), 0, 0.5 );
    g_free ( sz );
    
    gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                       0, 4, iRow, iRow + 1,
                       GTK_EXPAND | GTK_FILL, 
                       GTK_EXPAND | GTK_FILL, 
                       8, 0 );
    
    iRow++;

    switch ( pes->et ) {

    case EVAL_EVAL:

      pw = gtk_label_new ( OutputPercents ( aarOutput[ 0 ], TRUE ) );
      gtk_misc_set_alignment( GTK_MISC( pw ), 0.5, 0.5 );
      
      gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                         0, 4, iRow, iRow + 1, 
                         GTK_EXPAND | GTK_FILL, 
                         GTK_EXPAND | GTK_FILL, 
                         8, 4 );

      iRow++;

      break;

    case EVAL_ROLLOUT:

      /* FIXME: */

      break;

    default:

      assert ( FALSE );
      break;

    }

    if ( arDouble[ OUTPUT_TAKE ] < arDouble[ OUTPUT_DROP ] ) {
      ai[ 0 ] = OUTPUT_TAKE;
      ai[ 1 ] = OUTPUT_DROP;
    }
    else {
      ai[ 0 ] = OUTPUT_DROP;
      ai[ 1 ] = OUTPUT_TAKE;
    }
      

    for ( i = 0; i < 2; i++ ) {

      /* numbering */

      sz = g_strdup_printf ( "%d.", i + 1 );
      pw = gtk_label_new ( sz );
      gtk_misc_set_alignment( GTK_MISC( pw ), 0, 0.5 );
      g_free ( sz );

      gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                         0, 1, iRow, iRow + 1, 
                         GTK_EXPAND | GTK_FILL, 
                         GTK_EXPAND | GTK_FILL, 
                         8, 0 );

      /* label */

      pw = gtk_label_new ( gettext ( aszCube[ ai[ i ] ] ) );
      gtk_misc_set_alignment( GTK_MISC( pw ), 0, 0.5 );

      gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                         1, 2, iRow, iRow + 1, 
                         GTK_EXPAND | GTK_FILL, 
                         GTK_EXPAND | GTK_FILL, 
                         8, 0 );

      /* equity */

      if ( ! ci.nMatchTo || ( ci.nMatchTo && ! fOutputMWC ) )
        sz = g_strdup_printf ( "%+7.3f", -arDouble[ ai [ i ] ] );
      else
        sz = g_strdup_printf ( "%7.3f%%", 
                               100.0f * ( 1.0 - eq2mwc( arDouble[ ai[ i ] ], 
                                                        &ci ) ) );

      pw = gtk_label_new ( sz );
      gtk_misc_set_alignment( GTK_MISC( pw ), 1, 0.5 );
      g_free ( sz );

      gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                         2, 3, iRow, iRow + 1, 
                         GTK_EXPAND | GTK_FILL, 
                         GTK_EXPAND | GTK_FILL, 
                         8, 0 );

      /* difference */

      if ( i ) {
        
        if ( ! ci.nMatchTo || ( ci.nMatchTo && ! fOutputMWC ) )
          sz = g_strdup_printf ( "(%+7.3f)", 
                                 arDouble[ ai [ 0 ] ] - 
                                 arDouble[ ai [ i ] ] );
        else
          sz = g_strdup_printf ( "(%+7.3f%%)", 
                                 100.0f * eq2mwc( arDouble[ ai[ 0 ] ], 
                                                  &ci )-
                                 100.0f * eq2mwc( arDouble[ ai[ i ] ], &ci ) );

        pw = gtk_label_new ( sz );
        gtk_misc_set_alignment( GTK_MISC( pw ), 1, 0.5 );
        g_free ( sz );
        
        gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                           3, 4, iRow, iRow + 1, 
                           GTK_EXPAND | GTK_FILL, 
                           GTK_EXPAND | GTK_FILL, 
                           8, 0 );

      }

      iRow++;

    }

    /* proper cube action */

    pw = gtk_label_new ( _("Correct response: ") );
    gtk_misc_set_alignment( GTK_MISC( pw ), 0, 0.5 );
        
    gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                       0, 2, iRow, iRow + 1, 
                       GTK_EXPAND | GTK_FILL, 
                       GTK_EXPAND | GTK_FILL, 
                       8, 8 );

    switch ( cd ) {

    case DOUBLE_TAKE:
    case NODOUBLE_TAKE:
    case TOOGOOD_TAKE:
    case REDOUBLE_TAKE:
    case NO_REDOUBLE_TAKE:
    case TOOGOODRE_TAKE:
    case NODOUBLE_DEADCUBE:
    case NO_REDOUBLE_DEADCUBE:
    case OPTIONAL_DOUBLE_TAKE:
    case OPTIONAL_REDOUBLE_TAKE:
      pw = gtk_label_new ( _("Take") );
      break;

    case DOUBLE_PASS:
    case TOOGOOD_PASS:
    case REDOUBLE_PASS:
    case TOOGOODRE_PASS:
    case OPTIONAL_DOUBLE_PASS:
    case OPTIONAL_REDOUBLE_PASS:
      pw = gtk_label_new ( _("Pass") );
      break;

    case DOUBLE_BEAVER:
    case NODOUBLE_BEAVER:
    case NO_REDOUBLE_BEAVER:
    case OPTIONAL_DOUBLE_BEAVER:
      pw = gtk_label_new ( _("Beaver!") );
      break;

    case NOT_AVAILABLE:
      pw = gtk_label_new ( _("Eat it!") );
      break;

    }

    gtk_misc_set_alignment( GTK_MISC( pw ), 0, 0.5 );
        
    gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                       2, 4, iRow, iRow + 1, 
                       GTK_EXPAND | GTK_FILL, 
                       GTK_EXPAND | GTK_FILL, 
                       8, 8 );

    return pwFrame;

}



/*
 * Make cube analysis widget 
 *
 * Input:
 *   aarOutput, aarStdDev: evaluations
 *   pes: evaluation setup
 *
 * Returns:
 *   nice and pretty widget with cube analysis
 *
 */

static GtkWidget *CubeAnalysis( float aarOutput[ 2 ][ NUM_ROLLOUT_OUTPUTS ],
                                float aarStdDev[ 2 ][ NUM_ROLLOUT_OUTPUTS ],
                                const evalsetup *pes,
                                matchstate *pms,
                                const int fDouble ) {

    cubeinfo ci;

    GtkWidget *pw;
    GtkWidget *pwTable;
    GtkWidget *pwFrame;

    int iRow;
    int i;
    cubedecision cd;
    float r;

    int ai[ 3 ];
    const char *aszCube[] = {
      NULL, 
      N_("No double"), 
      N_("Double, take"), 
      N_("Double, pass") 
    };

    float arDouble[ 4 ];
    gchar *sz;


    if( pes->et == EVAL_NONE )
	return NULL;

    GetMatchStateCubeInfo( &ci, pms );

    cd = FindCubeDecision ( arDouble, aarOutput, &ci );
    
    if( !GetDPEq( NULL, NULL, &ci ) && ! fDouble )
	/* No cube action possible */
	return NULL;

    /* header */

    pwFrame = gtk_frame_new ( _("Cube analysis") );
    gtk_container_set_border_width ( GTK_CONTAINER ( pwFrame ), 8 );

    pw = gtk_vbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER ( pwFrame ), pw );

    pwTable = gtk_table_new ( 8, 4, FALSE );
    gtk_box_pack_start ( GTK_BOX ( pw ), pwTable, FALSE, FALSE, 0 );

    iRow = 0;

    /* cubeless equity */

    if ( pes->et == EVAL_EVAL ) 
      sz = g_strdup_printf ( ( ! ci.nMatchTo || ( ci.nMatchTo && ! fOutputMWC ) ) ?
                             _("Cubeless %d-ply equity: %s") :
                             _("Cubeless %d-ply MWC: %s"), 
                             pes->ec.nPlies,
                             OutputEquity ( aarOutput[ 0 ][ OUTPUT_EQUITY ], &ci, TRUE ) );
    else 
      sz = g_strdup_printf ( ( ! ci.nMatchTo || ( ci.nMatchTo && ! fOutputMWC ) ) ?
                             _("Cubeless rollout equity: %s") :
                             _("Cubeless rollout MWC: %s"), 
                             OutputEquity ( aarOutput[ 0 ][ OUTPUT_EQUITY ], &ci, TRUE ) );
    
    pw = gtk_label_new ( sz );
    gtk_misc_set_alignment( GTK_MISC( pw ), 0, 0.5 );
    g_free ( sz );
    
    gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                       0, 4, iRow, iRow + 1,
                       GTK_EXPAND | GTK_FILL, 
                       GTK_EXPAND | GTK_FILL, 
                       8, 0 );
    
    iRow++;


    /* if EVAL_EVAL include cubeless equity and winning percentages */

    if ( pes->et == EVAL_EVAL ) {

      pw = gtk_label_new ( OutputPercents ( aarOutput[ 0 ], TRUE ) );
      gtk_misc_set_alignment( GTK_MISC( pw ), 0.5, 0.5 );
      
      gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                         0, 4, iRow, iRow + 1, 
                         GTK_EXPAND | GTK_FILL, 
                         GTK_EXPAND | GTK_FILL, 
                         8, 4 );

      iRow++;

    }

    getCubeDecisionOrdering ( ai, arDouble, aarOutput, &ci );

    for ( i = 0; i < 3; i++ ) {

      /* numbering */

      sz = g_strdup_printf ( "%d.", i + 1 );
      pw = gtk_label_new ( sz );
      gtk_misc_set_alignment( GTK_MISC( pw ), 0, 0.5 );
      g_free ( sz );

      gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                         0, 1, iRow, iRow + 1, 
                         GTK_EXPAND | GTK_FILL, 
                         GTK_EXPAND | GTK_FILL, 
                         8, 0 );

      /* label */

      pw = gtk_label_new ( gettext ( aszCube[ ai[ i ] ] ) );
      gtk_misc_set_alignment( GTK_MISC( pw ), 0, 0.5 );

      gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                         1, 2, iRow, iRow + 1, 
                         GTK_EXPAND | GTK_FILL, 
                         GTK_EXPAND | GTK_FILL, 
                         8, 0 );

      /* equity */

      sz = OutputEquity ( arDouble[ ai [ i ] ], &ci, TRUE );

      pw = gtk_label_new ( sz );
      gtk_misc_set_alignment( GTK_MISC( pw ), 1, 0.5 );

      gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                         2, 3, iRow, iRow + 1, 
                         GTK_EXPAND | GTK_FILL, 
                         GTK_EXPAND | GTK_FILL, 
                         8, 0 );

      /* difference */

      if ( i ) {
        
        pw = gtk_label_new ( OutputEquityDiff ( arDouble[ ai [ i ] ], 
                                                arDouble[ OUTPUT_OPTIMAL ], &ci ) );
        gtk_misc_set_alignment( GTK_MISC( pw ), 1, 0.5 );
        
        gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                           3, 4, iRow, iRow + 1, 
                           GTK_EXPAND | GTK_FILL, 
                           GTK_EXPAND | GTK_FILL, 
                           8, 0 );

      }

      iRow++;
      
      /* rollout details */
      
      if ( pes->et == EVAL_ROLLOUT && 
           ( ai[ i ] == OUTPUT_TAKE || ai[ i ] == OUTPUT_NODOUBLE ) ) {

        /* FIXME: output cubeless euqities and percentages for rollout */
        /*        probably along with rollout details */
        
        pw = gtk_label_new ( OutputPercents ( aarOutput[ ai [ i ] - 1 ], TRUE ) );
        gtk_misc_set_alignment( GTK_MISC( pw ), 0.5, 0.5 );
        
        gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                           0, 4, iRow, iRow + 1, 
                           GTK_EXPAND | GTK_FILL, 
                           GTK_EXPAND | GTK_FILL, 
                           8, 4 );

        iRow++;
        
      }
        
    }

    /* proper cube action */

    pw = gtk_label_new ( _("Proper cube action: ") );
    gtk_misc_set_alignment( GTK_MISC( pw ), 0, 0.5 );
        
    gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                       0, 2, iRow, iRow + 1, 
                       GTK_EXPAND | GTK_FILL, 
                       GTK_EXPAND | GTK_FILL, 
                       8, 8 );

    pw = gtk_label_new ( GetCubeRecommendation ( cd ) );
    gtk_misc_set_alignment( GTK_MISC( pw ), 0, 0.5 );
        
    gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                       2, 3, iRow, iRow + 1, 
                       GTK_EXPAND | GTK_FILL, 
                       GTK_EXPAND | GTK_FILL, 
                       8, 8 );

    /* percent */

    if ( ( r = getPercent ( cd, arDouble ) ) >= 0.0 ) {

      sz = g_strdup_printf ( "(%.1f%%)", 100.0 * r );
      pw = gtk_label_new ( sz );
      gtk_misc_set_alignment( GTK_MISC( pw ), 1, 0.5 );

      gtk_table_attach ( GTK_TABLE ( pwTable ), pw,
                         3, 4, iRow, iRow + 1, 
                         GTK_EXPAND | GTK_FILL, 
                         GTK_EXPAND | GTK_FILL, 
                         8, 8 );

    }

    return pwFrame;
}


static void
UpdateCubeAnalysis ( cubehintdata *pchd ) {

  GtkWidget *pw;

  switch ( pchd->mt ) {
  case MOVE_NORMAL:
  case MOVE_DOUBLE:
    pw = CubeAnalysis ( pchd->aarOutput,
                        pchd->aarStdDev,
                        pchd->pes,
                        &pchd->ms,
                        pchd->mt );
    break;

  case MOVE_DROP:
  case MOVE_TAKE:
    pw = TakeAnalysis ( pchd->mt,
                        pchd->aarOutput,
                        pchd->aarStdDev,
                        pchd->pes,
                        &pchd->ms );
    break;

  default:

    assert ( FALSE );
    break;

  }

  /* update cache */

  UpdateStoredCube ( pchd->aarOutput, pchd->aarStdDev, pchd->pes, &pchd->ms ),

  /* destroy current analysis */

  gtk_container_remove ( GTK_CONTAINER ( pchd->pw ),
                         pchd->pwFrame );

  /* re-create analysis + tools */

  gtk_box_pack_start ( GTK_BOX ( pchd->pw ), pchd->pwFrame = pw, 
                       FALSE, FALSE, 0 );


  gtk_widget_show_all ( pchd->pw );
  

}




static void
CubeAnalysisRollout ( GtkWidget *pw, cubehintdata *pchd ) {

  cubeinfo ci;
  float aarOutput[ 2 ][ NUM_ROLLOUT_OUTPUTS ];
  float aarStdDev[ 2 ][ NUM_ROLLOUT_OUTPUTS ];
  rolloutstat aarsStatistics[ 2 ][ 2 ];

  GetMatchStateCubeInfo( &ci, &pchd->ms );

  if ( GeneralCubeDecisionR ( "", aarOutput, aarStdDev, aarsStatistics,
                              pchd->ms.anBoard, &ci, &rcRollout ) < 0 ) {
    return;
  }
  
  memcpy ( pchd->aarOutput, aarOutput, 
           2 * NUM_ROLLOUT_OUTPUTS * sizeof ( float ) );
  memcpy ( pchd->aarStdDev, aarStdDev, 
           2 * NUM_ROLLOUT_OUTPUTS * sizeof ( float ) );

  pchd->pes->et = EVAL_ROLLOUT;
  memcpy ( &pchd->pes->rc, &rcRollout, sizeof ( rcRollout ) );

  if ( pchd->arDouble )
    FindCubeDecision ( pchd->arDouble, pchd->aarOutput, &ci );

  UpdateCubeAnalysis ( pchd );

}

static void
CubeAnalysisEval ( GtkWidget *pw, cubehintdata *pchd ) {

  cubeinfo ci;
  float aarOutput[ 2 ][ NUM_ROLLOUT_OUTPUTS ];

  GetMatchStateCubeInfo( &ci, &pchd->ms );
  
  ProgressStart( _("Considering cube action...") );

  if ( GeneralCubeDecisionE ( aarOutput, pchd->ms.anBoard, &ci, 
                              &esEvalCube.ec ) < 0 ) {
    ProgressEnd();
    return;
  }
  ProgressEnd();

  /* save evaluation */

  memcpy ( pchd->aarOutput, aarOutput, 
           2 * NUM_ROLLOUT_OUTPUTS * sizeof ( float ) );

  pchd->pes->et = EVAL_EVAL;
  memcpy ( &pchd->pes->ec, &esEvalCube.ec, sizeof ( evalcontext ) );
  if ( pchd->arDouble )
     FindCubeDecision ( pchd->arDouble, pchd->aarOutput, &ci );

  UpdateCubeAnalysis ( pchd );

}

static void
CubeAnalysisEvalSettings ( GtkWidget *pw, void *unused ) {

  SetEvalCube ( NULL, 0, NULL );

}

static void
CubeAnalysisRolloutSettings ( GtkWidget *pw, void *unused ) {

  SetRollouts ( NULL, 0, NULL );

}

static void
CubeAnalysisMWC ( GtkWidget *pw, cubehintdata *pchd ) {

  char sz[ 80 ];
  int f = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pw ) );

  if ( f != fOutputMWC ) {
    sprintf ( sz, "set output mwc %s", fOutputMWC ? "off" : "on" );
    
    UserCommand ( sz );
  }

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( pw ), fOutputMWC );

  UpdateCubeAnalysis ( pchd );

}


static char *
GetContent ( cubehintdata *pchd ) {

  static char *pc;
  cubeinfo ci;

  GetMatchStateCubeInfo ( &ci, &pchd->ms );

  pc = OutputCubeAnalysis ( pchd->aarOutput,
                            pchd->aarStdDev,
                            pchd->pes,
                            &ci,
                            FALSE, FALSE,
                            SKILL_NONE, SKILL_NONE );

  return pc;

}


static void
CubeAnalysisCopy ( GtkWidget *pw, cubehintdata *pchd ) {

#ifdef WIN32

  char *pc =  GetContent ( pchd );

  if ( pc )
    WinCopy ( pc );

#else /* WIN32 */

  /* claim primary selection and CLIPBOARD */

  gtk_selection_owner_set ( pw, gdk_atom_intern ("CLIPBOARD", FALSE), 
                            GDK_CURRENT_TIME );
  gtk_selection_owner_set ( pw, GDK_SELECTION_PRIMARY, 
                            GDK_CURRENT_TIME );

#endif /* ! WIN32 */

}

#ifndef WIN32

static void
CubeAnalysisGetSelection ( GtkWidget *pw, GtkSelectionData *psd,
                           guint n, guint t, cubehintdata *pchd ) {

  /* copy cube analysis to primary selection and CLIPBOARD */

  char *pc = GetContent ( pchd );

  if ( pc )
    gtk_selection_data_set( psd, GDK_SELECTION_TYPE_STRING, 8,
                            pc, strlen ( pc ) );

}

#endif /* WIN32 */


static GtkWidget *
CreateCubeAnalysisTools ( cubehintdata *pchd ) {


  GtkWidget *pwTools;
  GtkWidget *pwEval = gtk_button_new_with_label ( _("Eval") );
  GtkWidget *pwEvalSettings = gtk_button_new_with_label ( _("...") );
  GtkWidget *pwRollout = gtk_button_new_with_label( _("Rollout") );
  GtkWidget *pwRolloutSettings = gtk_button_new_with_label ( _("...") );
  GtkWidget *pwMWC = gtk_toggle_button_new_with_label( _("MWC") );
  GtkWidget *pwCopy = gtk_button_new_with_label ( _("Copy") );

  GtkTooltips *pt = gtk_tooltips_new ();

  /* toolbox on the left with buttons for eval, rollout and more */
  
  pchd->pwTools = pwTools = gtk_table_new (4, 2, FALSE);
  
  gtk_table_attach (GTK_TABLE (pwTools), pwEval, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  gtk_table_attach (GTK_TABLE (pwTools), pwEvalSettings, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  gtk_table_attach (GTK_TABLE (pwTools), pwRollout, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  gtk_table_attach (GTK_TABLE (pwTools), pwRolloutSettings, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  gtk_table_attach (GTK_TABLE (pwTools), pwMWC, 0, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  gtk_table_attach (GTK_TABLE (pwTools), pwCopy, 0, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  gtk_widget_set_sensitive( pwMWC, pchd->ms.nMatchTo );
  
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( pwMWC ),
                                 fOutputMWC );

  /* selection */

  gtk_selection_add_target( pwCopy, 
                            gdk_atom_intern ("CLIPBOARD", FALSE),
                            GDK_SELECTION_TYPE_STRING, 0 );
  gtk_selection_add_target( pwCopy, 
                            GDK_SELECTION_PRIMARY,
                            GDK_SELECTION_TYPE_STRING, 0 );

  /* signals */

  gtk_signal_connect( GTK_OBJECT( pwRollout ), "clicked",
                      GTK_SIGNAL_FUNC( CubeAnalysisRollout ), pchd );
  gtk_signal_connect( GTK_OBJECT( pwEval ), "clicked",
                      GTK_SIGNAL_FUNC( CubeAnalysisEval ), pchd );
  gtk_signal_connect( GTK_OBJECT( pwEvalSettings ), "clicked",
                      GTK_SIGNAL_FUNC( CubeAnalysisEvalSettings ), NULL );
  gtk_signal_connect( GTK_OBJECT( pwRolloutSettings ), "clicked",
                      GTK_SIGNAL_FUNC( CubeAnalysisRolloutSettings ), NULL );
  gtk_signal_connect( GTK_OBJECT( pwMWC ), "toggled",
                      GTK_SIGNAL_FUNC( CubeAnalysisMWC ), pchd );
  gtk_signal_connect( GTK_OBJECT( pwCopy ), "clicked",
                      GTK_SIGNAL_FUNC( CubeAnalysisCopy ), pchd );
#ifndef WIN32
  gtk_signal_connect( GTK_OBJECT( pwCopy ), "selection_get",
                      GTK_SIGNAL_FUNC( CubeAnalysisGetSelection ), pchd );
#endif /* WIN32 */

  /* tool tips */

  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( pt ), pwRollout,
                         _("Rollout cube decision"),
                         _("Rollout cube decision") );

  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( pt ), pwEval,
                         _("Evaluate cube decision"),
                         _("Evaluate cube decision") );

  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( pt ), pwRolloutSettings,
                         _("Modify rollout settings"),
                         _("Modify rollout settings") );

  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( pt ), pwEvalSettings,
                         _("Modify evaluation settings"),
                         _("Modify evaluation settings") );

  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( pt ), pwMWC,
                         _("Toggle output as MWC or equity"),
                         _("Toggle output as MWC or equity") );

  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( pt ), pwCopy,
                         _("Copy"),
                         _("Copy") );

  /* FIXME: work in progress */
  // gtk_widget_set_sensitive ( pwCopy, FALSE );

  return pwTools;
  
}


extern GtkWidget *
CreateCubeAnalysis ( float aarOutput[ 2 ][ NUM_ROLLOUT_OUTPUTS ],
                     float aarStdDev[ 2 ][ NUM_ROLLOUT_OUTPUTS ],
                     float arDouble[],
                     evalsetup *pes,
                     const movetype mt ) {

  cubehintdata *pchd = (cubehintdata *) malloc ( sizeof ( cubehintdata ) );

  GtkWidget *pw;
  GtkWidget *pwx;

  pchd->aarOutput = aarOutput;
  pchd->aarStdDev = aarStdDev;
  pchd->arDouble = arDouble;
  pchd->pes = pes;
  pchd->mt = mt;
  pchd->ms = ms;

  pchd->pw = pw = gtk_hbox_new ( 4, FALSE );

  switch ( mt ) {

  case MOVE_NORMAL:
  case MOVE_DOUBLE:

    pchd->pwFrame = CubeAnalysis ( aarOutput, aarStdDev, pes, 
                                   &pchd->ms,
                                   mt == MOVE_DOUBLE );
    break;

  case MOVE_TAKE:
  case MOVE_DROP:

    pchd->pwFrame = TakeAnalysis ( mt, 
                                   aarOutput,
                                   aarStdDev,
                                   pes,
                                   &pchd->ms );
    break;

  default:

    assert ( FALSE );
    break;

  }

  if ( ! pchd->pwFrame )
    return NULL;

  gtk_box_pack_start ( GTK_BOX ( pw ), pchd->pwFrame, FALSE, FALSE, 0 );


  pwx = gtk_hbox_new ( 0, FALSE );

  gtk_box_pack_start ( GTK_BOX ( pwx ), pw, FALSE, FALSE, 0 );

  gtk_box_pack_start ( GTK_BOX ( pwx ), 
                     CreateCubeAnalysisTools ( pchd ), 
                     FALSE, FALSE, 0 );

  gtk_object_set_data_full( GTK_OBJECT( pw ), "user_data", 
                            pchd, free );


  return pwx;


}

