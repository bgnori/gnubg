/*
 * gtkchequer.c
 *
 * by Joern Thyssen <jthyssen@dk.ibm.com>, 2002
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
 * $Id: gtkchequer.c,v 1.86 2008/01/07 20:16:04 Superfly_Jon Exp $
 */

#include "config.h"

#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "backgammon.h"
#include "drawboard.h"
#include "eval.h"
#include "positionid.h"
#include "rollout.h"
#include "gtkboard.h"
#include "gtkgame.h"
#include "gtkchequer.h"
#include <glib/gi18n.h>
#include "gtktempmap.h"
#include "gtkwindows.h"
#include "progress.h"
#include "format.h"
#if USE_BOARD3D
#include "fun3d.h"
#endif

int showMoveListDetail = 1;
moverecord *pmrCurAnn;

static void MoveListRolloutClicked(GtkWidget *pw, hintdata *phd)
{
	cubeinfo ci;
	int i, c, res;
	move *m;
	int *ai;
	void *p;
	GList *pl, *plSelList = MoveListGetSelectionList(phd);
	if (!plSelList)
		return;

	GetMatchStateCubeInfo(&ci, &ms);

	c = g_list_length(plSelList);

  /* setup rollout dialog */
  {
	move **ppm = (move**)malloc( c * sizeof (move *));
	cubeinfo** ppci = (cubeinfo**)malloc( c * sizeof (cubeinfo *));
	char (*asz)[40] = (char(*)[40])malloc(40 * c);

  for( i =  0, pl = plSelList; i < c; pl = pl->next, i++ )
  {
    m = ppm[ i ] = MoveListGetMove(phd, pl);
    ppci[ i ] = &ci;
    FormatMove ( asz[ i ], msBoard(), m->anMove );
  }
	MoveListFreeSelectionList(plSelList);

	GTKSetCurrentParent(pw);
  RolloutProgressStart( &ci, c, NULL, &rcRollout, asz, &p );

  if ( fAction )
    HandleXAction();

  res = ScoreMoveRollout ( ppm, (const cubeinfo**)ppci, c, RolloutProgress, p );

  RolloutProgressEnd( &p );

	free(asz);
	free(ppm);
	free(ppci);

	if (res < 0)
		return;

  /* Calling RefreshMoveList here requires some extra work, as
     it may reorder moves */
  MoveListUpdate ( phd );
  }

  MoveListClearSelection(0, 0, phd);

  ai = (int *) malloc ( phd->pml->cMoves * sizeof ( int ) );
  RefreshMoveList ( phd->pml, ai );

  if ( phd->piHighlight && phd->pml->cMoves ) 
    *phd->piHighlight = ai [ *phd->piHighlight ];

  free ( ai );

  MoveListUpdate ( phd );
}

extern void ShowMove ( hintdata *phd, const int f )
{
  char *sz;
  TanBoard anBoard;
	if ( f )
	{
		move *pm;
		GList *plSelList = MoveListGetSelectionList(phd);
		if (!plSelList)
			return;

		/* the button is toggled */
		pm = MoveListGetMove(phd, plSelList);

		MoveListFreeSelectionList(plSelList);

		memcpy ( anBoard, msBoard(), sizeof(TanBoard) );
		ApplyMove ( anBoard, pm->anMove, FALSE );

		UpdateMove( ( BOARD( pwBoard ) )->board_data, anBoard );
	}
  else {

    sz = g_strdup ( "show board" );
    UserCommand( sz );
    g_free ( sz );

  }
#if USE_BOARD3D
	RestrictiveRedraw();
#endif
}

static void MoveListTempMapClicked( GtkWidget *pw, hintdata *phd )
{
  GList *pl;
  char szMove[ 100 ];
  matchstate *ams;
  int i, c;
  gchar **asz;

	GList *plSelList = MoveListGetSelectionList(phd);
	if (!plSelList)
	    return;

	c = g_list_length(plSelList);

  ams = (matchstate *) g_malloc( c * sizeof ( matchstate ) );
  asz = (char **) g_malloc ( c * sizeof ( char * ) );

  for( i = 0, pl = plSelList; pl; pl = pl->next, ++i ) {
  
    move *m = MoveListGetMove(phd, pl);

    /* Apply move to get board */

    memcpy( &ams[ i ], &ms, sizeof ( matchstate ) );

    FormatMove( szMove, (ConstTanBoard)ams[ i ].anBoard, m->anMove );
    ApplyMove( ams[ i ].anBoard, m->anMove, FALSE );

    /* Swap sides */

    SwapSides( ams[ i ].anBoard );
    ams[ i ].fMove = ! ams[ i ].fMove;
    ams[ i ].fTurn = ! ams[ i ].fTurn;

    /* Show temp map dialog */

    asz[ i ] = g_strdup( szMove );

  }
	MoveListFreeSelectionList(plSelList);

	GTKSetCurrentParent(pw);
  GTKShowTempMap( ams, c, ( const gchar** ) asz, TRUE );

  g_free( ams );
  for ( i = 0; i < c; ++i )
    g_free( asz[ i ] );
  g_free( asz );
}

static void
MoveListMWC ( GtkWidget *pw, hintdata *phd )
{
  char sz[ 80 ];
  int f = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pw ) );

  if ( f != fOutputMWC ) {
    sprintf ( sz, "set output mwc %s", fOutputMWC ? "off" : "on" );
    
    UserCommand ( sz );
  }

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( pw ), fOutputMWC );

  MoveListUpdate ( phd );

}

static void
EvalMoves ( hintdata *phd, evalcontext *pec )
{
  GList *pl;
  cubeinfo ci;
  int *ai;
	GList *plSelList = MoveListGetSelectionList(phd);

	if (!plSelList)
		return;

  GetMatchStateCubeInfo( &ci, &ms );
  
  ProgressStart( _("Evaluating positions...") );

	for(pl = plSelList; pl; pl = pl->next)
	{
    if ( ScoreMove (NULL, MoveListGetMove(phd, pl), &ci, pec, pec->nPlies ) < 0 ) {
      ProgressEnd ();
		MoveListFreeSelectionList(plSelList);
      return;
    }

    /* Calling RefreshMoveList here requires some extra work, as
       it may reorder moves */

    MoveListUpdate ( phd );

  }
	MoveListFreeSelectionList(plSelList);

  MoveListClearSelection(0, 0, phd);

  ai = (int *) malloc ( phd->pml->cMoves * sizeof ( int ) );
  RefreshMoveList ( phd->pml, ai );

  if ( phd->piHighlight && phd->pml->cMoves ) 
    *phd->piHighlight = ai [ *phd->piHighlight ];

  free ( ai );

  MoveListUpdate ( phd );

  ProgressEnd ();
}

static void
MoveListEval ( GtkWidget *pw, hintdata *phd )
{
  EvalMoves ( phd, &esEvalChequer.ec );
}

static void
MoveListEvalPly ( GtkWidget *pw, hintdata *phd )
{
  char *szPly = (char*)gtk_object_get_data ( GTK_OBJECT ( pw ), "user_data" );
#if defined (REDUCTION_CODE)
  evalcontext ec = { TRUE, 0, 0, TRUE, 0.0 };
#else
  evalcontext ec = { TRUE, 0, TRUE, TRUE, 0.0 };
#endif
  /* Reset interrupt flag */
  fInterrupt = FALSE;

  ec.nPlies = atoi ( szPly );

  EvalMoves ( phd, &ec );
}

static void
MoveListEvalSettings ( GtkWidget *pw, void *unused )
{
	GTKSetCurrentParent(pw);
  SetEvaluation ( NULL, 0, NULL );

  /* bring the dialog holding this button to the top */
  gtk_window_present ( GTK_WINDOW ( gtk_widget_get_toplevel( pw ) ) );
}

static void
MoveListRolloutSettings ( GtkWidget *pw, void *unused )
{
	GTKSetCurrentParent(pw);
	SetRollouts ( NULL, 0, NULL );

  /* bring the dialog holding this button to the top */
  gtk_window_present ( GTK_WINDOW ( gtk_widget_get_toplevel( pw ) ) );
}

static void MoveListRolloutPresets(GtkWidget * pw, hintdata * phd)
{
	const gchar *preset;
	gchar *file=NULL;
	gchar *path=NULL;
	gchar *command=NULL;

	preset = (const gchar *) gtk_object_get_data(GTK_OBJECT(pw), "user_data");
	file = g_strdup_printf("%s.rol", preset);
	path = g_build_filename(szHomeDirectory, "rol", file, NULL);
	if (g_file_test(path, G_FILE_TEST_IS_REGULAR)) {
		command = g_strdup_printf("load commands \"%s\"", path);
		outputoff();
		UserCommand(command);
		outputon();
		MoveListRolloutClicked(pw, phd);
	}
	else
	{
		outputerrf(_("You need to save a preset as \"%s\""), file);
		MoveListRolloutSettings(pw, NULL);
	}
        g_free(file);
        g_free(path);
	g_free(command);
}

typedef int ( *cfunc )( const void *, const void * );

static int CompareInts( int *p0, int *p1 ) {

    return *p0 - *p1;
}

static char *MoveListCopyData ( hintdata *phd )
{
	int c, i;
	GList *pl;
	char *sz, *pch;
	int *an;
	GList *plSelList = MoveListGetSelectionList(phd);
	c = g_list_length(plSelList);

	an = (int*)malloc( c * sizeof( an[ 0 ] ) );
	sz = (char*)malloc( c * 9 * 80 );

	*sz = 0;

	for (i = 0, pl = plSelList; pl; pl = pl->next, i++)
  {
    move *m = MoveListGetMove(phd, pl);
    int rank = 0;
    while(&phd->pml->amMoves[rank] != m)
      rank++;
    an[i] = rank;
	}

	MoveListFreeSelectionList(plSelList);

	qsort( an, c, sizeof( an[ 0 ] ), (cfunc) CompareInts );

	for( i = 0, pch = sz; i < c; i++, pch = strchr( pch, 0 ) )
		FormatMoveHint( pch, &ms, phd->pml, an[ i ], TRUE, TRUE, TRUE );

	free( an );

	return sz;
}

static void
MoveListMove ( GtkWidget *pw, hintdata *phd )
{
	move m;
	move *pm;
	char szMove[ 40 ];
	TanBoard anBoard;
	GList *plSelList = MoveListGetSelectionList(phd);
	if (!plSelList)
		return;
  
	pm = MoveListGetMove(phd, plSelList);
	MoveListFreeSelectionList(plSelList);

	memcpy(&m, pm, sizeof(move));

	memcpy ( anBoard, msBoard(), sizeof(TanBoard) );
	ApplyMove ( anBoard, m.anMove, FALSE );

	if ( ! ms.fMove )
	SwapSides ( anBoard );

	sprintf ( szMove, "show fullboard %s", PositionID ( (ConstTanBoard)anBoard ) );
	UserCommand ( szMove );

	if ( phd->fDestroyOnMove )
	/* Destroy widget on exit */
		gtk_widget_destroy( gtk_widget_get_toplevel( pw ) );

	FormatMove( szMove, msBoard(), m.anMove );
	UserCommand( szMove );

#if USE_BOARD3D
	RestrictiveRedraw();
#endif
}

static void
MoveListDetailsClicked( GtkWidget *pw, hintdata *phd )
{
	 showMoveListDetail = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pw));
	 /* Reshow list */
	 SetAnnotation(pmrCurAnn);
}

GtkWidget *pwDetails;

static void MoveListCopy(GtkWidget *pw, hintdata *phd)
{
  char *pc = MoveListCopyData(phd);
  if ( pc )
    TextToClipboard( pc );
}

static GtkWidget *
CreateMoveListTools ( hintdata *phd )
{
  GtkWidget *pwTools;
  GtkWidget *pwEval = gtk_button_new_with_label ( _("Eval") );
  GtkWidget *pwEvalSettings = gtk_button_new_with_label ( _("...") );
  GtkWidget *pwRollout = gtk_button_new_with_label( _("Rollout") );
  GtkWidget *pwRolloutSettings = gtk_button_new_with_label ( _("...") );
  GtkWidget *pwMWC = gtk_toggle_button_new_with_label( _("MWC") );
  GtkWidget *pwMove = gtk_button_new_with_label ( _("Move") );
  GtkWidget *pwShow = gtk_toggle_button_new_with_label ( _("Show") );
  GtkWidget *pwCopy = gtk_button_new_with_label ( _("Copy") );
  GtkWidget *pwTempMap = gtk_button_new_with_label( _("Temp. Map") );
  GtkWidget *pwply;
  int i;
  char *sz;

  GtkTooltips *pt = gtk_tooltips_new ();

  pwDetails = 
    phd->fDetails ? NULL : gtk_toggle_button_new_with_label( _("Details") );
  phd->pwRollout = pwRollout;
  phd->pwRolloutSettings = pwRolloutSettings;
  phd->pwEval = pwEval;
  phd->pwEvalSettings = pwEvalSettings;
  phd->pwMove = pwMove;
  phd->pwShow = pwShow;
  phd->pwCopy = pwCopy;
  phd->pwTempMap = pwTempMap;

  /* toolbox on the left with buttons for eval, rollout and more */
  
  pwTools = gtk_table_new (2, phd->fDetails ? 6 : 7, FALSE);
  
  gtk_table_attach (GTK_TABLE (pwTools), pwEval, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  gtk_table_attach (GTK_TABLE (pwTools), pwEvalSettings, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  phd->pwEvalPly = gtk_hbox_new ( FALSE, 0 );
  gtk_table_attach (GTK_TABLE (pwTools), phd->pwEvalPly, 2, 3, 0, 1, 
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  for ( i = 0; i < 5; ++i ) {

    sz = g_strdup_printf ( "%d", i ); /* string is freed by set_data_full */
    pwply = gtk_button_new_with_label ( sz );

    gtk_box_pack_start ( GTK_BOX ( phd->pwEvalPly ), pwply, TRUE, TRUE, 0 );

    g_signal_connect( G_OBJECT( pwply ), "clicked",
                        G_CALLBACK( MoveListEvalPly ), phd );

    gtk_object_set_data_full ( GTK_OBJECT ( pwply ), "user_data", sz, g_free );

    sz = g_strdup_printf ( _("Evaluate play on cubeful %d-ply"), i );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( pt ), pwply, sz, sz );
    g_free ( sz );

  }

  gtk_table_attach (GTK_TABLE (pwTools), pwShow, 3, 4, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  gtk_table_attach (GTK_TABLE (pwTools), pwMWC, 4, 5, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  if ( !phd->fDetails ) 
    gtk_table_attach (GTK_TABLE (pwTools), pwDetails, 5, 6, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);

  gtk_table_attach (GTK_TABLE (pwTools), pwRollout, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  gtk_table_attach (GTK_TABLE (pwTools), pwRolloutSettings, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  phd->pwRolloutPresets = gtk_hbox_new ( FALSE, 0 );
  gtk_table_attach (GTK_TABLE (pwTools), phd->pwRolloutPresets, 2, 3, 1, 2, 
                   (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);

  for ( i = 0; i < 5; ++i ) {
	  GtkWidget *ro_preset;
	  sz = g_strdup_printf ( "%c", i+'a' ); /* string is freed by set_data_full */
	  ro_preset = gtk_button_new_with_label ( sz );

	  gtk_box_pack_start ( GTK_BOX ( phd->pwRolloutPresets ), ro_preset, TRUE, TRUE, 0 );

	  g_signal_connect( G_OBJECT( ro_preset ), "clicked",
			  G_CALLBACK( MoveListRolloutPresets ), phd );

	  gtk_object_set_data_full ( GTK_OBJECT ( ro_preset ), "user_data", sz, g_free );

	  sz = g_strdup_printf ( _("Rollout preset %c"), i+'a' );
	  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( pt ), ro_preset, sz, sz );
	  g_free ( sz );

  }

  gtk_table_attach (GTK_TABLE (pwTools), pwMove, 3, 4, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  

  gtk_table_attach (GTK_TABLE (pwTools), pwCopy, 4, 5, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  gtk_table_attach (GTK_TABLE (pwTools), pwTempMap, 5, 6, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  
  gtk_widget_set_sensitive( pwMWC, ms.nMatchTo );
  gtk_widget_set_sensitive( pwMove, FALSE );
  gtk_widget_set_sensitive( pwCopy, FALSE );
  gtk_widget_set_sensitive( pwTempMap, FALSE );
  
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( pwMWC ),
                                 fOutputMWC );

  if (pwDetails)
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pwDetails), showMoveListDetail);
  /* signals */

  g_signal_connect( G_OBJECT( pwRollout ), "clicked",
                      G_CALLBACK( MoveListRolloutClicked ), phd );
  g_signal_connect( G_OBJECT( pwEval ), "clicked",
                      G_CALLBACK( MoveListEval ), phd );
  g_signal_connect( G_OBJECT( pwEvalSettings ), "clicked",
                      G_CALLBACK( MoveListEvalSettings ), NULL );
  g_signal_connect( G_OBJECT( pwRolloutSettings ), "clicked",
                      G_CALLBACK( MoveListRolloutSettings ), NULL );
  g_signal_connect( G_OBJECT( pwMWC ), "toggled",
                      G_CALLBACK( MoveListMWC ), phd );
  g_signal_connect( G_OBJECT( pwMove ), "clicked",
                      G_CALLBACK( MoveListMove ), phd );
  g_signal_connect( G_OBJECT( pwShow ), "toggled",
                      G_CALLBACK( MoveListShowToggledClicked ), phd );
  g_signal_connect( G_OBJECT( pwCopy ), "clicked",
                      G_CALLBACK( MoveListCopy ), phd );
  g_signal_connect( G_OBJECT( pwTempMap ), "clicked",
                      G_CALLBACK( MoveListTempMapClicked ), phd );
  if ( !phd->fDetails )
    g_signal_connect( G_OBJECT( pwDetails ), "clicked",
                        G_CALLBACK( MoveListDetailsClicked ), phd );

  /* tool tips */

  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( pt ), pwRollout,
                         _("Rollout chequer play with current settings"),
                         _("Rollout chequer play with current settings") );

  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( pt ), pwEval,
                         _("Evaluate chequer play with current settings"),
                         _("Evaluate chequer play with current settings") );

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
                         _("Copy selected moves to clipboard"),
                         _("Copy selected moves to clipboard") );

  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( pt ), pwMove,
                         _("Move the selected move"),
                         _("Move the selected move") );

  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( pt ), pwTempMap,
                         _("Show Sho Sengoku Temperature Map of position "
                           "after selected move"),
                         "" );


  return pwTools;
}

extern void HintDoubleClick(GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       hintdata *phd)
{
	gtk_button_clicked( GTK_BUTTON( phd->pwMove ) );
}

extern void HintSelect(GtkTreeSelection *selection, hintdata *phd)
{
	CheckHintButtons( phd );
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(phd->pwShow)))
	{
		int c = gtk_tree_selection_count_selected_rows(selection);
		switch (c)
		{
		case 0:
		case 1:
			ShowMove(phd, c);
		break;

		default:
			ShowMove(phd, FALSE);
		break;
		}
	}
}

extern int 
CheckHintButtons( hintdata *phd )
{
    int c = g_list_length(MoveListGetSelectionList(phd));

    gtk_widget_set_sensitive( phd->pwMove, c == 1 && phd->fButtonsValid );
    gtk_widget_set_sensitive( phd->pwCopy, c && phd->fButtonsValid );
    gtk_widget_set_sensitive( phd->pwTempMap, c && phd->fButtonsValid );
    gtk_widget_set_sensitive( phd->pwRollout, c && phd->fButtonsValid );
    gtk_widget_set_sensitive( phd->pwRolloutPresets, c && phd->fButtonsValid );
    gtk_widget_set_sensitive( phd->pwEval, c && phd->fButtonsValid );
    gtk_widget_set_sensitive( phd->pwEvalPly, c && phd->fButtonsValid );

    return c;
}

extern GtkWidget *
CreateMoveList( movelist *pml, unsigned int *piHighlight, const int fButtonsValid,
                const int fDestroyOnMove, const int fDetails )
{
    GtkWidget *pw;
    GtkWidget *pwHBox, *mlt;

    hintdata *phd = (hintdata *) malloc ( sizeof ( hintdata ) );

    /* This function should only be called when the game state matches
       the move list. */
    g_assert( ms.fMove == 0 || ms.fMove == 1 );

    phd->piHighlight = piHighlight;
    phd->pml = pml;
    phd->fButtonsValid = fButtonsValid;
    phd->fDestroyOnMove = fDestroyOnMove;
    phd->pwMove = NULL;
    phd->fDetails = fDetails;
  mlt = CreateMoveListTools( phd );
	MoveListCreate(phd);

    pw = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( pw ),
                                    GTK_POLICY_NEVER, 
                                    GTK_POLICY_AUTOMATIC );
    gtk_container_add(GTK_CONTAINER(pw), phd->pwMoves);

    pwHBox = gtk_vbox_new ( FALSE, 0 );  /* Variable name does not match 
					    actual widget */

    gtk_box_pack_start ( GTK_BOX ( pwHBox ), pw, TRUE, TRUE, 0 );
    gtk_box_pack_end ( GTK_BOX ( pwHBox ),
                       mlt,
                       FALSE, FALSE, 0 );

    gtk_object_set_data_full( GTK_OBJECT( pwHBox ), "user_data", 
                              phd, free );

	CheckHintButtons(phd);

    return pwHBox;
}
