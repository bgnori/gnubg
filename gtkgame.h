/*
 * gtkgame.h
 *
 * by Gary Wong <gtw@gnu.org>, 2000, 2002.
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
 * $Id: gtkgame.h,v 1.120 2007/10/23 10:40:17 c_anthon Exp $
 */

#ifndef _GTKGAME_H_
#define _GTKGAME_H_

#include <stdio.h>
#include <gtk/gtk.h>

#include "backgammon.h"
#include "rollout.h"
#include "relational.h"
#include "gtkpanels.h"
#include "gtkchequer.h"

extern GtkWidget* pwBoard;
extern int fX, fNeedPrompt;
extern unsigned int nDelay;
extern guint nNextTurn; /* GTK idle function */

extern GtkWidget *pwMain, *pwMenuBar;
extern GtkWidget *pwToolbar;
extern GtkTooltips *ptt;
extern GtkAccelGroup *pagMain;
extern GtkWidget *pwMessageText, *pwPanelVbox, *pwAnalysis, *pwCommentary;

extern GtkWidget *pwGrab;
extern GtkWidget *pwOldGrab;

extern int fEndDelay;

extern gboolean ShowGameWindow( void );

extern void GTKAddMoveRecord( moverecord *pmr );
extern void GTKPopMoveRecord( moverecord *pmr );
extern void GTKSetMoveRecord( moverecord *pmr );
extern void GTKClearMoveRecord( void );
extern void GTKAddGame( moverecord *pmr );
extern void GTKPopGame( int c );
extern void GTKSetGame( int i );
extern void GTKRegenerateGames( void );

extern void GTKFreeze( void );
extern void GTKThaw( void );

extern void GTKSuspendInput(void);
extern void GTKResumeInput(void);

extern void InitGTK( int *argc, char ***argv );
extern void RunGTK( GtkWidget *pwSplash, char *commands, char *python_script, char *match );
extern void GTKAllowStdin( void );
extern void GTKDisallowStdin( void );
extern void GTKDelay( void );
extern void ShowList( char *asz[], char *szTitle, GtkWidget* pwParent );
extern void GTKUpdateScores(void);

extern void GTKOutput( char *sz );
extern void GTKOutputErr( char *sz );
extern void GTKOutputX( void );
extern void GTKOutputNew( void );
extern void GTKProgressStart( char *sz );
extern void GTKProgress( void );
extern void GTKProgressEnd( void );
extern void
GTKProgressStartValue( char *sz, int iMax );
extern void
GTKProgressValue ( int fValue, int iMax );
extern void GTKBearoffProgress( int i );

extern void GTKDumpStatcontext( int game );
extern void GTKEval( char *szOutput );
extern void 
GTKHint( movelist *pmlOrig, const unsigned int iMove );
extern void GTKCubeHint( float aarOutput[ 2 ][ NUM_ROLLOUT_OUTPUTS ], 
			 float aarStdDev[ 2 ][ NUM_ROLLOUT_OUTPUTS ], 
			 const evalsetup *pes );

extern void GTKSet( void *p );
extern void GTKUpdateAnnotations( void );
extern int GTKGetManualDice( unsigned int an[ 2 ] );
extern void GTKShowVersion( void );
extern void GTKShowCalibration( void );
extern void *GTKCalibrationStart( void ),
    GTKCalibrationUpdate( void *context, float rEvalsPerSec ),
    GTKCalibrationEnd( void *context );
extern void GTKDumpRolloutResults(GtkWidget *widget, gpointer data);
extern void GTKWinCopy( GtkWidget *widget, gpointer data);
extern void GTKResign( gpointer *p, guint n, GtkWidget *pw);
extern void
GTKResignHint( float arOutput[], float rEqBefore, float rEqAfter,
               cubeinfo *pci, int fMWC );
extern void GTKSaveSettings( void );
extern void GTKSetCube( gpointer *p, guint n, GtkWidget *pw );
extern void GTKSetDice( gpointer *p, guint n, GtkWidget *pw );
extern void GTKHelp( char *sz );
extern void GTKMatchInfo( void );
extern void GTKShowBuildInfo(GtkWidget *pw, GtkWidget *pwParent);
extern void GTKCommandShowCredits(GtkWidget *pw, GtkWidget* parent);
extern void GTKShowScoreSheet(void);
extern void SwapBoardToPanel(int ToPanel);
extern void CommentaryChanged( GtkWidget *pw, GtkTextBuffer *buffer );

extern void SetEvaluation( gpointer *p, guint n, GtkWidget *pw );
extern void SetRollouts( gpointer *p, guint n, GtkWidget *pw );
extern void SetMET( GtkWidget *pw, gpointer p );

extern void HintMove( GtkWidget *pw, GtkWidget *pwMoves );

extern int fTTY;
extern int frozen;

extern int 
GtkTutor ( char *sz );

extern void GTKNew ( void );

extern void
RefreshGeometries ( void );

extern void
setWindowGeometry(gnubgwindow window);

extern int
GTKGetMove ( int anMove[ 8 ] );

extern void GTKRecordShow( FILE *pfIn, char *sz, char *szPlayer );

extern void
GTKTextToClipboard( const char *sz );

extern char *
GTKChangeDisk( const char *szMsg, const int fChange, 
               const char *szMissingFile );

extern int 
GTKReadNumber( char *szTitle, char *szPrompt, int nDefault,
               int nMin, int nMax, int nInc );

extern void Undo(void);


extern void SetToolbarStyle(int value);

extern void
GTKShowManual( void );

extern void GtkShowQuery(RowSet* pRow);


extern void GetStyleFromRCFile(GtkStyle** ppStyle, char* name, GtkStyle* psBase);
extern void ToggleDockPanels( gpointer *p, guint n, GtkWidget *pw );
extern void DisplayWindows(void);
extern int DockedPanelsShowing(void);
extern int ArePanelsShowing(void);
extern int ArePanelsDocked(void);
extern int IsPanelDocked(gnubgwindow window);
extern int GetPanelWidth(gnubgwindow panel);
extern void SetPanelWidget(gnubgwindow window, GtkWidget* pWin);
extern GtkWidget* GetPanelWidget(gnubgwindow window);
extern void PanelShow(gnubgwindow panel);
extern void PanelHide(gnubgwindow panel);
extern int IsPanelShowVar(gnubgwindow panel, void *p);
extern int SetMainWindowSize(void);
extern void ShowHidePanel(gnubgwindow panel);
extern void SetAnnotation( moverecord *pmr );
extern void GTKTextWindow( const char *szOutput, const char *title, const int type  );
extern void FullScreenMode(int state);
extern void GetFullscreenWindowSettings(int *panels, int *ids, int *maxed);
extern void GtkChangeLanguage(void);
extern void OK( GtkWidget *pw, int *pf );
extern int edit_new(int length);
extern char *ReturnHits( int anBoard[ 2 ][ 25 ] );

#if USE_BOARD3D
extern void SetSwitchModeMenuText(void);
extern gboolean gtk_gl_init_success;
#endif

extern gint NextTurnNotify( gpointer p );
extern void HandleXAction( void );
#if HAVE_LIBREADLINE
extern int fReadingCommand;
extern void ProcessInput( char* sz );
#endif
extern void UserCommand( char* sz );
extern void HideAllPanels ( gpointer *p, guint n, GtkWidget *pw );
extern void ShowAllPanels ( gpointer *p, guint n, GtkWidget *pw );
extern void GL_Freeze( void );
extern void GL_Thaw( void );
extern void GL_SetNames(void);
void DockPanels(void);
extern GtkWidget *StatsPixmapButton(GdkColormap *pcmap, char **xpm,
				void (*fn)());
extern GtkWidget* GL_Create(void);
extern GList *MoveListGetSelectionList(const hintdata *phd);
extern move *MoveListGetMove(const hintdata *phd, GList *pl);
extern void MoveListFreeSelectionList(GList *pl);
extern void MoveListUpdate ( const hintdata *phd );
extern gint MoveListClearSelection( GtkWidget *pw, GdkEventSelection *pes, hintdata *phd );
extern moverecord *pmrCurAnn;
extern void MoveListShowToggledClicked(GtkWidget *pw, hintdata *phd);
extern void MoveListCreate(hintdata *phd);
extern void HintDoubleClick(GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       hintdata *phd);
extern void HintSelect(GtkTreeSelection *selection, hintdata *phd);
extern GtkWidget *pwMoveAnalysis;
extern GdkColor wlCol;
extern GtkItemFactory *pif;
extern void ShowMove ( hintdata *phd, const int f );
extern GtkWidget *pom;
extern GtkWidget *hpaned;
#endif
