/*
 * text.c
 *
 * by Joern Thyssen  <jthyssen@dk.ibm.com>, 2002
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
 * $Id: text.c,v 1.46 2003/07/16 13:57:09 thyssen Exp $
 */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>

#include "backgammon.h"
#include "drawboard.h"
#include "export.h"
#include "eval.h"
#include "positionid.h"
#include "matchid.h"
#include "record.h"

#include "i18n.h"


/* "Color" of chequers */

static char *aszColorName[] = { "O", "X" };


extern char *
OutputRolloutResult ( const char *szIndent,
                    char asz[][ 1024 ],
                    float aarOutput[][ NUM_ROLLOUT_OUTPUTS ],
                    float aarStdDev[][ NUM_ROLLOUT_OUTPUTS ],
                    const cubeinfo aci[],
                    const int cci,
                    const int fCubeful ) {

  static char sz[ 1024 ];
  int ici;
  char *pc;

  strcpy ( sz, "" );

  for ( ici = 0; ici < cci; ici++ ) {

    /* header */

    if ( asz && *asz[ ici ] ) {
      if ( szIndent && *szIndent )
        strcat ( sz, szIndent );
      sprintf ( pc = strchr ( sz, 0 ), "%s:\n", asz[ ici ] );
    }

    /* output */

    if ( szIndent && *szIndent )
      strcat ( sz, szIndent );

    strcat ( sz, "  " );
    strcat ( sz, OutputPercents ( aarOutput[ ici ], TRUE ) );
    strcat ( sz, " CL " );
    strcat ( sz, OutputEquityScale ( aarOutput[ ici ][ OUTPUT_EQUITY ], 
                                     &aci[ ici ], &aci[ 0 ], TRUE ) );

    if ( fCubeful ) {
      strcat ( sz, " CF " );
      strcat ( sz, OutputMWC ( aarOutput[ ici ][ OUTPUT_CUBEFUL_EQUITY ], 
                               &aci[ 0 ], TRUE ) );
    }

    strcat ( sz, "\n" );

    /* std dev */

    if ( szIndent && *szIndent )
      strcat ( sz, szIndent );

    strcat ( sz, " [" );
    strcat ( sz, OutputPercents ( aarStdDev[ ici ], FALSE ) );
    strcat ( sz, " CL " );
    strcat ( sz, OutputEquityScale ( aarStdDev[ ici ][ OUTPUT_EQUITY ], 
                                     &aci[ ici ], &aci[ 0 ], FALSE ) );

    if ( fCubeful ) {
      strcat ( sz, " CF " );
      strcat ( sz, OutputMWC ( aarStdDev[ ici ][ OUTPUT_CUBEFUL_EQUITY ], 
                               &aci[ 0 ], FALSE ) );
    }

    strcat ( sz, "]\n" );

  }

  return sz;

}


extern char *
OutputEvalContext ( const evalcontext *pec, const int fChequer ) {

  static char sz[ 1024 ];
  char *pc;
  int i;
  
  sprintf ( sz, _("%d-ply %s"), 
            pec->nPlies, 
            ( ! fChequer || pec->fCubeful ) ? _("cubeful") : _("cubeless") );

  if ( pec->nPlies == 2 ) 
    sprintf ( pc = strchr ( sz, 0 ),
              " %d%% speed",
              (pec->nReduced) ? 100 / pec->nReduced : 100 );

  if ( fChequer && pec->nPlies ) {
    /* FIXME: movefilters!!! */
  }

  if ( pec->rNoise > 0.0f )
    sprintf ( pc = strchr ( sz, 0 ),
              ", noise %0.3g (%s)",
              pec->rNoise,
              pec->fDeterministic ? "d" : "nd" );

  for ( i = 0; i < NUM_SETTINGS; i++ )

    if ( ! cmp_evalcontext ( &aecSettings[ i ], pec ) ) {
      sprintf ( pc = strchr ( sz, 0 ),
                " [%s]",
                gettext ( aszSettings[ i ] ) );
      break;
    }

  return sz;

}



/* return -1 if this isn't a predefined setting
           n = index of predefined setting if it's a match or
               if the eval context matches and it's 0 ply
 */
extern int
GetPredefinedChequerplaySetting( const evalcontext *pec,
                                 movefilter aamf[ MAX_FILTER_PLIES ][ MAX_FILTER_PLIES ] ) {

  int nEval;
  int nFilter;
  int nPlies = pec->nPlies;
  int i;
  int fSame;
  int nPreset;
  int Accept;

  for (nEval = 0; nEval < NUM_SETTINGS; ++nEval) {
    if (cmp_evalcontext( aecSettings + nEval, pec) == 0) {
      /* eval matches and it's 0 ply, we have a predefined one */
      if (nPlies == 0)
	return nEval;

      /* see if there's a filter set which matches and uses the
	 same eval context (world class and supremo use the same
         eval context and different filters) */
      for (nFilter = 0; nFilter < NUM_SETTINGS; ++nFilter) {
	/* see what filter set goes with the predefined settings */
	if ((nPreset = aiSettingsMoveFilter[nFilter]) < 0)
	  continue;

	/* nPreset = 0/tiny, 1/normal, 2/large, 3/huge */
	fSame = 1;
	for (i = 0; i < nPlies - 1; ++i) {
	  if ((Accept = aamf[ nPlies - 1 ][ i ].Accept) !=
	      aaamfMoveFilterSettings[nPreset][nPlies - 1 ][ i ].Accept) {
	    fSame = 0;
	    break;
	  }

	  /* if they are both ignore this level, don't check 
	     the extra and threshold */
	  if (Accept < 0)
	    continue;

	  if (aamf[ nPlies - 1 ][ i ].Extra !=
	      aaamfMoveFilterSettings[nPreset][nPlies - 1 ][ i ].Extra) {
	    fSame = 0;
	    break;
	  }

	  if (fabs (aamf[ nPlies - 1 ][ i ].Threshold - 
	   aaamfMoveFilterSettings[nPreset][nPlies - 1 ][ i ].Threshold)
	      > 0.1e-6) {
	    fSame = 0;
	    break;
	  }
	}
	/* the filters match (for this nPlies, so we may have a 
	   preset */
	if (fSame && (nFilter == nEval))
	  return nEval;
      }
    }
  }

  return -1;
}

extern char *
OutputMoveFilterPly( const char *szIndent, 
                     const int nPlies,
                     movefilter aamf[ MAX_FILTER_PLIES ][ MAX_FILTER_PLIES ] ) {

  static char sz[ 1024 ];
  int i;

  strcpy( sz, "" );

  if ( ! nPlies ) 
    return sz;
    
  for ( i = 0; i < nPlies; ++i ) {

    movefilter *pmf = &aamf[ nPlies - 1 ][ i ];

    if ( szIndent && *szIndent )
      strcat( sz, szIndent );

    if ( pmf->Accept < 0 ) {
      sprintf( strchr( sz, 0 ), 
               _("Skip pruning for %d-ply moves.\n"),
               i );
      continue;
    }

    if ( pmf->Accept == 1 )
      sprintf( strchr( sz, 0 ),
               _("keep the best %d-ply move"), i );
    else
      sprintf( strchr( sz, 0 ),
               _("keep the first %d %d-ply moves"),
               pmf->Accept, i );

    if ( pmf->Extra )
      sprintf( strchr( sz, 0 ),
               _(" and up to %d more moves within equity %0.3g\n"),
               pmf->Extra, pmf->Threshold );
    else
      strcat( sz, "\n" );

  }

  return sz;

}
                                 
static void
OutputEvalContextsForRollout( char *sz, const char *szIndent,
                              const evalcontext aecCube[ 2 ],
                              const evalcontext aecChequer[ 2 ],
                              movefilter aaamf[ 2 ][ MAX_FILTER_PLIES ][ MAX_FILTER_PLIES ] ) {

  int fCube = ! cmp_evalcontext ( &aecCube[ 0 ], &aecCube[ 1 ] );
  int fChequer = ! cmp_evalcontext ( &aecChequer[ 0 ], &aecChequer[ 1 ] );
  int fMovefilter = fChequer && 
    ( !aecChequer[ 0 ].nPlies || 
      equal_movefilter( aecChequer[ 0 ].nPlies - 1,
                        aaamf[ 0 ][ aecChequer[ 0 ].nPlies - 1 ],
                        aaamf[ 1 ][ aecChequer[ 0 ].nPlies - 1 ] ) );
  int fIdentical = fCube && fChequer && fMovefilter;
  int i;
  int j;

  for ( i = 0; i < 1 + ! fIdentical; i++ ) {

    if ( ! fIdentical ) {

      if ( szIndent && *szIndent )
        strcat ( sz, szIndent );

      sprintf ( strchr ( sz, 0 ), _("Player %d:\n" ), i );
    }

    /* chequer play */

    j = GetPredefinedChequerplaySetting( &aecChequer[ i ], aaamf[ i ] );

    if ( szIndent && *szIndent )
      strcat ( sz, szIndent );

    if ( aecChequer[ i ].nPlies ) {

      sprintf( strchr( sz, 0 ),
               _("Play: %s "), 
               ( j < 0 ) ? "" : gettext ( aszSettings[ j ] ) );

      strcat( sz, OutputEvalContext ( &aecChequer[ i ], TRUE ) );
      strcat( sz, "\n" );
      strcat( sz, OutputMoveFilterPly( szIndent, aecChequer[ i ].nPlies, 
                                       aaamf[ i ] ) );

    }
    else {
      strcat ( sz, _("Play: ") );
      strcat ( sz, OutputEvalContext ( &aecChequer[ i ], FALSE ) );
      strcat ( sz, "\n" );
    }

    if ( szIndent && *szIndent )
      strcat ( sz, szIndent );

    strcat ( sz, _("Cube: ") );
    strcat ( sz, OutputEvalContext ( &aecCube[ i ], FALSE ) );
    strcat ( sz, "\n" );

  }

}



extern char *
OutputRolloutContext ( const char *szIndent, const evalsetup *pes ) {

  const rolloutcontext *prc = &pes->rc;
  static char sz[ 1024 ];

  strcpy ( sz, "" );

  if ( szIndent && *szIndent )
    strcat ( sz, szIndent );

  if ( prc->nTruncate && prc->fDoTruncate)
    sprintf ( strchr ( sz, 0 ),
              prc->fCubeful ?
              _("Truncated cubeful rollout (depth %d)") :
              _("Truncated cubeless rollout (depth %d)"), 
              prc->nTruncate );
  else
    sprintf ( strchr ( sz, 0 ),
              prc->fCubeful ? 
              _("Full cubeful rollout") :
              _("Full cubeless rollout") );

  if ( prc->fTruncBearoffOS && ! prc->fCubeful )
    strcat ( sz, _(" (trunc. at one-sided bearoff)") );
  else if ( prc->fTruncBearoff2 && ! prc->fCubeful )
    strcat ( sz, _(" (trunc. at exact bearoff)") );

  sprintf ( strchr ( sz, 0 ),
            prc->fVarRedn ? _(" with var.redn.") : _(" without var.redn.") );

  strcat ( sz, "\n" );
  
  if ( szIndent && *szIndent )
    strcat ( sz, szIndent );

  sprintf ( strchr ( sz, 0 ),
            "%d games",
            prc->nGamesDone );

  if ( prc->fInitial )
    strcat ( sz, ", rollout as initial position" );

  if ( prc->fRotate ) 
    sprintf ( strchr ( sz, 0 ),
              _(", %s dice gen. with seed %d and quasi-random dice\n"),
              gettext( aszRNG[ prc->rngRollout ] ),
              prc->nSeed );
  else
    sprintf ( strchr ( sz, 0 ),
              _(", %s dice generator with seed %d\n"),
              gettext( aszRNG[ prc->rngRollout ] ),
              prc->nSeed );

  /* stop on std.err */

  if ( prc->fStopOnSTD )
    sprintf( strchr( sz, 0 ),
             _("Stop when std.errs. are small enough: ratio "
               "%.4g (min. %d games)\n"),
             prc->rStdLimit, prc->nMinimumGames );

  /* first play */

  OutputEvalContextsForRollout( sz, szIndent, 
                                prc->aecCube, prc->aecChequer,
                                ((rolloutcontext *)prc)->aaamfChequer );

  /* later play */

  if ( prc->fLateEvals ) {

    if ( szIndent && *szIndent )
      strcat ( sz, szIndent );
    
    sprintf( strchr( sz, 0 ), 
             _("Different evaluations after %d plies:\n"),
             prc->nLate );

    OutputEvalContextsForRollout( sz, szIndent, 
                                  prc->aecCubeLate, prc->aecChequerLate,
                                  ((rolloutcontext *)prc)->aaamfLate );


  }

  return sz;

}

/*
 * Return formatted string with equity or MWC.
 *
 * Input:
 *    r: equity (either money equity for normalised money eq. for match play
 *    pci: cubeinfo
 *    f: indicates equity (TRUE) or std. error (FALSE)
 *    
 *
 * Important: function is not re-entrant. Caller must save output
 * if needed.
 */

extern char *
OutputEquity ( const float r, const cubeinfo *pci, const int f ) {

  static char sz[ 9 ];

  if ( !pci->nMatchTo || ( pci->nMatchTo && ! fOutputMWC ) )
    sprintf ( sz, f ? "%+7.3f" : "%7.3f", r );
  else {
    if ( fOutputMatchPC )
      sprintf ( sz, "%6.2f%%", 
                100.0f * ( f ? eq2mwc ( r, pci ) : se_eq2mwc ( r, pci ) ) );
    else
      sprintf ( sz, "%6.4f", 
                f ? eq2mwc ( r, pci ) : se_eq2mwc ( r, pci ) );
  }

  return sz;

}


extern char *
OutputMoneyEquity ( const float ar[], const int f ) {

  static char sz[ 9 ];

  sprintf ( sz, f ? "%+7.3f" : "%7.3f", 
            2.0 * ar[ OUTPUT_WIN ] - 1.0
            + ar[ OUTPUT_WINGAMMON ] + ar[ OUTPUT_WINBACKGAMMON] 
            - ar[ OUTPUT_LOSEGAMMON ] - ar[ OUTPUT_LOSEBACKGAMMON] );

  return sz;

}



/*
 * Return formatted string with equity or MWC.
 *
 * Input:
 *    r: equity (either money equity for normalised money eq. for match play
 *    pci: cubeinfo
 *    f: indicates equity (TRUE) or std. error (FALSE)
 *    
 *
 * Important: function is not re-entrant. Caller must save output
 * if needed.
 */

extern char *
OutputEquityScale ( const float r, const cubeinfo *pci, 
                    const cubeinfo *pciBase, const int f ) {

  static char sz[ 9 ];

  if ( ! pci->nMatchTo ) 
    sprintf ( sz, f ? "%+7.3f" : "%7.3f", pci->nCube / pciBase->nCube * r );
  else {

    if ( fOutputMWC ) {

      if ( fOutputMatchPC )
        sprintf ( sz, "%6.2f%%", 
                  100.0f * ( f ? eq2mwc ( r, pci ) : se_eq2mwc ( r, pci ) ) );
      else
        sprintf ( sz, "%6.4f", 
                  f ? eq2mwc ( r, pci ) : se_eq2mwc ( r, pci ) );

    }
    else
      sprintf ( sz, f ? "%+7.3f" : "%7.3f", 
                mwc2eq ( eq2mwc ( r, pci ), pciBase ) );


  }

  return sz;

}


/*
 * Return formatted string with equity or MWC for an equity difference.
 *
 * Input:
 *    r: equity (either money equity for normalised money eq. for match play
 *    pci: cubeinfo
 *    f: indicates equity (TRUE) or std. error (FALSE)
 *    
 *
 * Important: function is not re-entrant. Caller must save output
 * if needed.
 */

extern char *
OutputEquityDiff ( const float r1, const float r2, const cubeinfo *pci ) {

  static char sz[ 9 ];

  if ( !pci->nMatchTo || ( pci->nMatchTo && ! fOutputMWC ) )
    sprintf ( sz, "%+7.3f", r1 - r2 );
  else {
    if ( fOutputMatchPC )
      sprintf ( sz, "%+6.2f%%", 
                100.0f * eq2mwc ( r1, pci ) - 100.0f * eq2mwc ( r2, pci ) );
    else
      sprintf ( sz, "%+6.4f", 
                eq2mwc ( r1, pci ) - eq2mwc ( r2, pci ) );
  }

  return sz;

}

/*
 * Return formatted string with equity or MWC.
 *
 * Input is: equity for money game/MWC for match play.
 *
 * Important: function is not re-entrant. Caller must save output
 * if needed.
 */


extern char *
OutputMWC ( const float r, const cubeinfo *pci, const int f ) {

  static char sz[ 9 ];

  if ( ! pci->nMatchTo ) 
    sprintf ( sz, "%+7.3f", r );
  else {
    
    if ( ! fOutputMWC ) 
      sprintf ( sz, "%+7.3f", 
                f ? mwc2eq ( r, pci ) : se_mwc2eq ( r, pci ) );
    else if ( fOutputMatchPC )
      sprintf ( sz, "%6.2f%%", 100.0f * r );
    else
      sprintf ( sz, "%6.4f", r );
  }

  return sz;

}


extern char *
OutputPercent ( const float r ) {

  static char sz[ 9 ];

  if ( fOutputWinPC )
    sprintf ( sz, "%5.1f%%", 100.0 * r );
  else
    sprintf ( sz, "%5.3f", r );

  return sz;

}

extern char *
OutputPercents ( const float ar[], const int f ) {

  static char sz[ 80 ];

  strcpy ( sz, "" );

  strcat ( sz, OutputPercent ( ar [ OUTPUT_WIN ] ) );
  strcat ( sz, " " );
  strcat ( sz, OutputPercent ( ar [ OUTPUT_WINGAMMON ] ) );
  strcat ( sz, " " );
  strcat ( sz, OutputPercent ( ar [ OUTPUT_WINBACKGAMMON ] ) );
  strcat ( sz, " - " );
  if ( f )
    strcat ( sz, OutputPercent ( 1.0f - ar [ OUTPUT_WIN ] ) );
  else
    strcat ( sz, OutputPercent ( ar [ OUTPUT_WIN ] ) );
  strcat ( sz, " " );
  strcat ( sz, OutputPercent ( ar [ OUTPUT_LOSEGAMMON ] ) );
  strcat ( sz, " " );
  strcat ( sz, OutputPercent ( ar [ OUTPUT_LOSEBACKGAMMON ] ) );

  return sz;

}

static void
printTextBoard ( FILE *pf, const matchstate *pms ) {

  int anBoard[ 2 ][ 25 ];
  char szBoard[ 2048 ];
  char sz[ 32 ], szCube[ 32 ], szPlayer0[ 35 ], szPlayer1[ 35 ],
    szScore0[ 35 ], szScore1[ 35 ], szMatch[ 35 ];
  char *apch[ 7 ] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };
  int anPips[ 2 ];

  memcpy ( anBoard, pms->anBoard, sizeof ( anBoard ) );

  apch[ 0 ] = szPlayer0;
  apch[ 6 ] = szPlayer1;

  if ( pms->anScore[ 0 ] == 1 )
    sprintf( apch[ 1 ] = szScore0, _("%d point"), pms->anScore[ 0 ] );
  else
    sprintf( apch[ 1 ] = szScore0, _("%d points"), pms->anScore[ 0 ] );

  if ( pms->anScore[ 1 ] == 1 )
    sprintf( apch[ 5 ] = szScore1, _("%d point"), pms->anScore[ 1 ] );
  else
    sprintf( apch[ 5 ] = szScore1, _("%d points"), pms->anScore[ 1 ] );

  if( pms->fDoubled ) {
    apch[ pms->fTurn ? 4 : 2 ] = szCube;

    sprintf( szPlayer0, "O: %s", ap[ 0 ].szName );
    sprintf( szPlayer1, "X: %s", ap[ 1 ].szName );
    sprintf( szCube, _("Cube offered at %d"), pms->nCube << 1 );
  } else {
    sprintf( szPlayer0, "O: %s", ap[ 0 ].szName );
    sprintf( szPlayer1, "X: %s", ap[ 1 ].szName );

    apch[ pms->fMove ? 4 : 2 ] = sz;
	
    if( pms->anDice[ 0 ] )
      sprintf( sz, 
               _("Rolled %d%d"), pms->anDice[ 0 ], pms->anDice[ 1 ] );
    else if( !GameStatus( anBoard, pms->bgv ) )
      strcpy( sz, _("On roll") );
    else
      sz[ 0 ] = 0;
	    
    if( pms->fCubeOwner < 0 ) {
      apch[ 3 ] = szCube;

      if( pms->nMatchTo )
        sprintf( szCube, 
                 _("%d point match (Cube: %d)"), pms->nMatchTo,
                 pms->nCube );
      else
        sprintf( szCube, _("(Cube: %d)"), pms->nCube );
    } else {
      int cch = strlen( ap[ pms->fCubeOwner ].szName );
		
      if( cch > 20 )
        cch = 20;
		
      sprintf( szCube, _("%c: %*s (Cube: %d)"), pms->fCubeOwner ? 'X' :
               'O', cch, ap[ pms->fCubeOwner ].szName, pms->nCube );

      apch[ pms->fCubeOwner ? 6 : 0 ] = szCube;

      if( pms->nMatchTo )
        sprintf( apch[ 3 ] = szMatch, _("%d point match"),
                 pms->nMatchTo );
    }
  }
    
  if( pms->fResigned )
    sprintf( strchr( sz, 0 ), _(", resigns %s"),
             gettext ( aszGameResult[ pms->fResigned - 1 ] ) );
	
  if( !pms->fMove )
    SwapSides( anBoard );
	

  fputs ( DrawBoard( szBoard, anBoard, pms->fMove, apch,
                     MatchIDFromMatchState ( pms ), anChequers[ ms.bgv ] ),
          pf);

  PipCount ( anBoard, anPips );

  fprintf ( pf, "Pip counts: O %d, X %d\n\n",
            anPips[ 0 ], anPips[ 1 ] );

}


/*
 * Print html header for board: move or cube decision 
 *
 * Input:
 *   pf: output file
 *   ms: current match state
 *   iMove: move no.
 *
 */

static void 
TextBoardHeader ( FILE *pf, const matchstate *pms, 
                  const int iGame, const int iMove ) {

  if ( iMove >= 0 )
    fprintf ( pf, _("Move number %d: "), iMove + 1 );

  if ( pms->fResigned ) 
    
    /* resignation */

    fprintf ( pf,
              _("%s resigns %d points\n\n"), 
              aszColorName[ pms->fTurn ],
              pms->fResigned * pms->nCube
            );
  
  else if ( pms->anDice[ 0 ] && pms->anDice[ 1 ] )

    /* chequer play decision */

    fprintf ( pf,
              _("%s to play %d%d\n\n"),
              aszColorName[ pms->fTurn ],
              pms->anDice[ 0 ], pms->anDice[ 1 ] 
            );

  else if ( pms->fDoubled )

    /* take decision */

    fprintf ( pf,
              _("%s doubles to %d\n\n"),
              aszColorName[ pms->fMove ],
              pms->nCube * 2
            );

  else

    /* cube decision */

    fprintf ( pf,
              _("%s on roll, cube decision?\n\n"),
              aszColorName[ pms->fTurn ]
            );


}


/*
 * Print html header: dtd, head etc.
 *
 * Input:
 *   pf: output file
 *   ms: current match state
 *
 */

static void 
TextPrologue ( FILE *pf, const matchstate *pms, const int iGame ) {

  if ( pms->nMatchTo )
    fprintf ( pf,
              _("%s (O, %d pts) vs. %s (X, %d pts) (Match to %d)\n\n"),
              ap [ 0 ].szName, pms->anScore[ 0 ],
              ap [ 1 ].szName, pms->anScore[ 1 ],
              pms->nMatchTo );
  else
    fprintf ( pf,
              _("%s (O, %d pts) vs. %s (X, %d pts) (money game)\n\n"),
              ap [ 0 ].szName, pms->anScore[ 0 ],
              ap [ 1 ].szName, pms->anScore[ 1 ] );

  fprintf ( pf, 
            _("Game number %d\n\n"), iGame + 1 );

}


/*
 * Print html header: dtd, head etc.
 *
 * Input:
 *   pf: output file
 *   ms: current match state
 *
 */

static void 
TextEpilogue ( FILE *pf, const matchstate *pms ) {

  time_t t;

  const char szVersion[] = "$Revision: 1.46 $";
  int iMajor, iMinor;

  iMajor = atoi ( strchr ( szVersion, ' ' ) );
  iMinor = atoi ( strchr ( szVersion, '.' ) + 1 );

  time ( &t );

  fprintf ( pf, 
            _("Output generated %s"
              "by GNU Backgammon %s ") ,
            ctime ( &t ), VERSION );
            
  fprintf ( pf,
            _("(Text Export version %d.%d)\n\n"),
            iMajor, iMinor );

}


/*
 * Print cube analysis
 *
 * Input:
 *  pf: output file
 *  arDouble: equitites for cube decisions
 *  fPlayer: player who doubled
 *  esDouble: eval setup
 *  pci: cubeinfo
 *  fDouble: double/no double
 *  fTake: take/drop
 *
 */

extern char *
OutputCubeAnalysis ( float aarOutput[ 2 ][ NUM_ROLLOUT_OUTPUTS ],
                     float aarStdDev[ 2 ][ NUM_ROLLOUT_OUTPUTS ],
                     evalsetup *pes, cubeinfo *pci,
                     int fDouble, int fTake,
                     skilltype stDouble,
                     skilltype stTake ) {

  const char *aszCube[] = {
    NULL, 
    N_("No double"), 
    N_("Double, take"), 
    N_("Double, pass") 
  };

  int i;
  int ai[ 3 ];
  float r;

  int fClose, fMissed;
  int fAnno = FALSE;

  float arDouble[ 4 ];

  static char sz[ 4096 ];
  char *pc;

  cubedecision cd;

  strcpy ( sz, "" );

  /* check if cube analysis should be printed */

  if ( pes->et == EVAL_NONE ) return NULL; /* no evaluation */
  if ( ! GetDPEq ( NULL, NULL, pci ) ) return NULL; /* cube not available */

  FindCubeDecision ( arDouble, aarOutput, pci );

  /* print alerts */

  fClose = isCloseCubedecision ( arDouble ); 
  fMissed = 
    fDouble > -1 && isMissedDouble ( arDouble, aarOutput, fDouble, pci );

  /* print alerts */

  if ( fMissed ) {

    fAnno = TRUE;

    /* missed double */

    sprintf ( pc = strchr ( sz, 0 ), "%s (%s)!",
              _("Alert: missed double"),
              OutputEquityDiff ( arDouble[ OUTPUT_NODOUBLE ], 
                                 ( arDouble[ OUTPUT_TAKE ] > 
                                   arDouble[ OUTPUT_DROP ] ) ? 
                                 arDouble[ OUTPUT_DROP ] : 
                                 arDouble[ OUTPUT_TAKE ],
                                 pci ) );
    
    if ( stDouble != SKILL_NONE )
      sprintf ( pc = strchr ( sz, 0 ), " [%s]", 
                gettext ( aszSkillType[ stDouble ] ) );
    
  }

  r = arDouble[ OUTPUT_TAKE ] - arDouble[ OUTPUT_DROP ];

  if ( fTake > 0 && r > 0.0f ) {

    fAnno = TRUE;

    /* wrong take */

    sprintf ( pc = strchr ( sz, 0 ), "%s (%s)!",
              _("Alert: wrong take"),
              OutputEquityDiff ( arDouble[ OUTPUT_DROP ],
                                 arDouble[ OUTPUT_TAKE ],
                                 pci ) );

    if ( stTake != SKILL_NONE )
      sprintf ( pc = strchr ( sz, 0 ), " [%s]", 
                gettext ( aszSkillType[ stTake ] ) );
    
  }

  r = arDouble[ OUTPUT_DROP ] - arDouble[ OUTPUT_TAKE ];

  if ( fDouble > 0 && ! fTake && r > 0.0f ) {

    fAnno = TRUE;

    /* wrong pass */

    sprintf ( pc = strchr ( sz, 0 ), "%s (%s)!",
              _("Alert: wrong pass"),
              OutputEquityDiff ( arDouble[ OUTPUT_TAKE ],
                                 arDouble[ OUTPUT_DROP ],
                                 pci ) );

    if ( stTake != SKILL_NONE )
      sprintf ( pc = strchr ( sz, 0 ), " [%s]", 
                gettext ( aszSkillType[ stTake ] ) );
    
  }


  if ( arDouble[ OUTPUT_TAKE ] > arDouble[ OUTPUT_DROP ] )
    r = arDouble[ OUTPUT_NODOUBLE ] - arDouble[ OUTPUT_DROP ];
  else
    r = arDouble[ OUTPUT_NODOUBLE ] - arDouble[ OUTPUT_TAKE ];

  if ( fDouble > 0 && r > 0.0f ) {

    fAnno = TRUE;

    /* wrong double */

    sprintf ( pc = strchr ( sz, 0 ), "%s (%s)!",
              _("Alert: wrong double"),
              OutputEquityDiff ( ( arDouble[ OUTPUT_TAKE ] > 
                                   arDouble[ OUTPUT_DROP ] ) ? 
                                 arDouble[ OUTPUT_DROP ] : 
                                 arDouble[ OUTPUT_TAKE ],
                                 arDouble[ OUTPUT_NODOUBLE ], 
                                 pci ) );

    if ( stDouble != SKILL_NONE )
      sprintf ( pc = strchr ( sz, 0 ), " [%s]", 
                gettext ( aszSkillType[ stDouble ] ) );

  }

  if ( ( stDouble != SKILL_NONE || stTake != SKILL_NONE ) && ! fAnno ) {
    
    if ( stDouble != SKILL_NONE ) {
      sprintf ( pc = strchr ( sz, 0 ), _("Alert: double decision marked %s"),
                gettext ( aszSkillType[ stDouble ] ) );
      strcat ( sz, "\n" );
    }

    if ( stTake != SKILL_NONE ) {
      sprintf ( pc = strchr ( sz, 0 ), _("Alert: take decision marked %s"),
                gettext ( aszSkillType[ stTake ] ) );
      strcat ( sz, "\n" );
    }

  }

  /* header */

  strcat ( sz, _("\nCube analysis\n") );

  /* ply & cubeless equity */

  switch ( pes->et ) {
  case EVAL_NONE:
    strcat ( sz, _("n/a") );
    break;
  case EVAL_EVAL:
    sprintf ( pc = strchr ( sz, 0 ), 
              _("%d-ply"), 
              pes->ec.nPlies );
    break;
  case EVAL_ROLLOUT:
    strcat ( sz, _("Rollout") );
    break;
  }

  if ( pci->nMatchTo ) 
    sprintf ( pc = strchr ( sz, 0 ), " %s %s (%s: %s)\n",
              ( !pci->nMatchTo || ( pci->nMatchTo && ! fOutputMWC ) ) ?
              _("cubeless equity") : _("cubeless MWC"),
              OutputEquity ( aarOutput[ 0 ][ OUTPUT_EQUITY ], pci, TRUE ),
              _("Money"), 
              OutputMoneyEquity ( aarOutput[ 0 ], TRUE ) );
  else
    sprintf ( pc = strchr ( sz, 0 ), " cubeless equity %s\n",
              OutputMoneyEquity ( aarOutput[ 0 ], TRUE ) );




  /* Output percentags for evaluations */

  if ( exsExport.fCubeDetailProb && pes->et == EVAL_EVAL ) {

    strcat ( sz, "  " );
    strcat ( sz, OutputPercents ( aarOutput[ 0 ], TRUE ) );

  }

  strcat( sz, "\n" );

  /* equities */

  strcat( sz, _("Cubeful equities:\n") );
  if ( pes->et == EVAL_EVAL && exsExport.afCubeParameters[ 0 ] ) {
    strcat( sz, "  " );
    strcat( sz, OutputEvalContext( &pes->ec, FALSE ) );
    strcat( sz, "\n" );
  }

  getCubeDecisionOrdering ( ai, arDouble, aarOutput, pci );

  for ( i = 0; i < 3; i++ ) {

    sprintf ( pc = strchr ( sz, 0 ),
              "%d. %-20s", i + 1, 
              gettext ( aszCube[ ai[ i ] ] ) );

    strcat ( sz, OutputEquity ( arDouble[ ai [ i ] ], pci, TRUE ) );

    if ( i )
      sprintf ( pc = strchr ( sz, 0 ),
                "  (%s)",
                OutputEquityDiff ( arDouble[ ai [ i ] ], 
                                   arDouble[ OUTPUT_OPTIMAL ], pci ) );
    strcat ( sz, "\n" );

  }

  /* cube decision */

  cd = FindBestCubeDecision ( arDouble, aarOutput, pci );

  sprintf ( pc = strchr ( sz, 0 ),
            "%s %s",
            _("Proper cube action:"),
            GetCubeRecommendation ( cd ) );

  if ( ( r = getPercent ( cd, arDouble ) ) >= 0.0 )
    sprintf ( pc = strchr ( sz, 0 ), " (%.1f%%)", 100.0f * r );

  strcat ( sz, "\n" );

  /* dump rollout */

  if ( pes->et == EVAL_ROLLOUT && exsExport.fCubeDetailProb ) {

    char asz[ 2 ][ 1024 ];
    cubeinfo aci[ 2 ];

    for ( i = 0; i < 2; i++ ) {

      memcpy ( &aci[ i ], pci, sizeof ( cubeinfo ) );

      if ( i ) {
        aci[ i ].fCubeOwner = ! pci->fMove;
        aci[ i ].nCube *= 2;
      }

      FormatCubePosition ( asz[ i ], &aci[ i ] );

    }

    strcat ( sz, _("Rollout details:\n") );

    strcat ( sz, 
             OutputRolloutResult ( NULL,
                                   asz,
                                   aarOutput, aarStdDev,
                                   aci, 2, pes->rc.fCubeful ) );
             

  }

  if ( pes->et == EVAL_ROLLOUT && exsExport.afCubeParameters[ 1 ] )
    strcat ( sz, OutputRolloutContext ( NULL, pes ) );
    
  return sz;


}


/*
 * Print cube analysis
 *
 * Input:
 *  pf: output file
 *  arDouble: equitites for cube decisions
 *  fPlayer: player who doubled
 *  esDouble: eval setup
 *  pci: cubeinfo
 *  fDouble: double/no double
 *  fTake: take/drop
 *
 */

static void
TextPrintCubeAnalysisTable ( FILE *pf, float arDouble[],
                             float aarOutput[ 2 ][ NUM_ROLLOUT_OUTPUTS ],
                             float aarStdDev[ 2 ][ NUM_ROLLOUT_OUTPUTS ],
                             int fPlayer,
                             evalsetup *pes, cubeinfo *pci,
                             int fDouble, int fTake,
                             skilltype stDouble,
                             skilltype stTake ) {

  int fActual, fClose, fMissed;
  int fDisplay;

  /* check if cube analysis should be printed */

  if ( pes->et == EVAL_NONE ) return; /* no evaluation */
  if ( ! GetDPEq ( NULL, NULL, pci ) ) return; /* cube not available */

  FindCubeDecision ( arDouble, aarOutput, pci );

  fActual = fDouble;
  fClose = isCloseCubedecision ( arDouble ); 
  fMissed = isMissedDouble ( arDouble, aarOutput, fDouble, pci );

  fDisplay = 
    ( fActual && exsExport.afCubeDisplay[ EXPORT_CUBE_ACTUAL ] ) ||
    ( fClose && exsExport.afCubeDisplay[ EXPORT_CUBE_CLOSE ] ) ||
    ( fMissed && exsExport.afCubeDisplay[ EXPORT_CUBE_MISSED ] ) ||
    ( exsExport.afCubeDisplay[ stDouble ] ) ||
    ( exsExport.afCubeDisplay[ stTake ] );

  if ( ! fDisplay )
    return;

  fputs ( OutputCubeAnalysis ( aarOutput,
                               aarStdDev,
                               pes,
                               pci,
                               fDouble, fTake, 
                               stDouble, stTake ),
          pf );

}



/*
 * Wrapper for print cube analysis
 *
 * Input:
 *  pf: output file
 *  pms: match state
 *  pmr: current move record
 *  szImageDir: URI to images
 *  szExtension: extension of images
 *  fTake: take/drop
 *
 */

static void
TextPrintCubeAnalysis ( FILE *pf, matchstate *pms, moverecord *pmr ) {

  cubeinfo ci;

  GetMatchStateCubeInfo ( &ci, pms );

  switch ( pmr->mt ) {

  case MOVE_NORMAL:

    /* cube analysis from move */

    TextPrintCubeAnalysisTable ( pf, pmr->n.arDouble, 
                                 pmr->n.aarOutput, pmr->n.aarStdDev,
                                 pmr->n.fPlayer,
                                 &pmr->n.esDouble, &ci, FALSE, -1,
                                 pmr->n.stCube, SKILL_NONE );

    break;

  case MOVE_DOUBLE:

    TextPrintCubeAnalysisTable ( pf, pmr->d.arDouble, 
                                 pmr->d.aarOutput, pmr->d.aarStdDev,
                                 pmr->d.fPlayer,
                                 &pmr->d.esDouble, &ci, TRUE, -1,
                                 pmr->d.st, SKILL_NONE );

    break;

  case MOVE_TAKE:
  case MOVE_DROP:

    /* cube analysis from double, {take, drop, beaver} */

    TextPrintCubeAnalysisTable ( pf, pmr->d.arDouble, 
                                 pmr->d.aarOutput, pmr->d.aarStdDev,
                                 pmr->d.fPlayer,
                                 &pmr->d.esDouble, &ci, TRUE, 
                                 pmr->mt == MOVE_TAKE,
                                 SKILL_NONE, /* FIXME: skill from prev. cube */
                                 pmr->d.st );

    break;

  default:

    assert ( FALSE );


  }

  return;

}


/*
 * Print move analysis
 *
 * Input:
 *  pf: output file
 *  pms: match state
 *  pmr: current move record
 *  szImageDir: URI to images
 *  szExtension: extension of images
 *
 */

static void
TextPrintMoveAnalysis ( FILE *pf, matchstate *pms, moverecord *pmr ) {

  char szBuf[ 1024 ];
  char sz[ 64 ];
  int i;

  cubeinfo ci;

  GetMatchStateCubeInfo( &ci, pms );

  /* check if move should be printed */

  if ( ! exsExport.afMovesDisplay [ pmr->n.stMove ] )
    return;

  /* print alerts */

  if ( pmr->n.stMove <= SKILL_BAD || pmr->n.stMove > SKILL_NONE ) {

    /* blunder or error */

    fprintf ( pf, 
              _("Alert: %s move"),
              gettext ( aszSkillType[ pmr->n.stMove ] ) );
    
    if ( !pms->nMatchTo || ( pms->nMatchTo && ! fOutputMWC ) )
      fprintf ( pf, " (%+7.3f)\n",
                pmr->n.ml.amMoves[ pmr->n.iMove ].rScore -
                pmr->n.ml.amMoves[ 0 ].rScore  );
    else
      fprintf ( pf, " (%+6.3f%%)\n",
                100.0f *
                eq2mwc ( pmr->n.ml.amMoves[ pmr->n.iMove ].rScore, &ci ) - 
                100.0f *
                eq2mwc ( pmr->n.ml.amMoves[ 0 ].rScore, &ci ) );

  }

  if ( pmr->n.lt != LUCK_NONE ) {

    /* joker */

    fprintf ( pf, 
              _("Alert: %s roll!"),
              gettext ( aszLuckType[ pmr->n.lt ] ) );
    
    if ( !pms->nMatchTo || ( pms->nMatchTo && ! fOutputMWC ) )
      fprintf ( pf, " (%+7.3f)\n", pmr->n.rLuck );
    else
      fprintf ( pf, " (%+6.3f%%)\n",
                100.0f * eq2mwc ( pmr->n.rLuck, &ci ) - 
                100.0f * eq2mwc ( 0.0f, &ci ) );

  }

  fputs ( "\n", pf );

  fprintf( pf, _("Rolled %d%d"), pmr->n.anRoll[ 0 ], pmr->n.anRoll[ 1 ] );

  if( pmr->n.rLuck != ERR_VAL )
    fprintf( pf, " (%s):\n", GetLuckAnalysis( pms, pmr->n.rLuck ) );
  else
    fprintf( pf, ":" );

  if ( pmr->n.ml.cMoves ) {
	
    for( i = 0; i < pmr->n.ml.cMoves; i++ ) {
      if( i >= exsExport.nMoves && i != pmr->n.iMove )
        continue;

      fputc( i == pmr->n.iMove ? '*' : ' ', pf );
      fputs( FormatMoveHint( szBuf, pms, &pmr->n.ml, i,
                             i != pmr->n.iMove ||
                             i != pmr->n.ml.cMoves - 1 ||
                             pmr->n.ml.cMoves == 1,
                             exsExport.fMovesDetailProb,
                             exsExport.afMovesParameters 
                             [ pmr->n.ml.amMoves[ i ].esMove.et - 1 ] ), 
             pf );


    }

  }
  else {

    if ( pmr->n.anMove[ 0 ] >= 0 )
      /* no movelist saved */
      fprintf ( pf,
                "*    %s\n",
                FormatMove ( sz, pms->anBoard, pmr->n.anMove ) );
    else 
      /* no legal moves */
      /* FIXME: output equity?? */
      fprintf ( pf,
                "*    %s\n",
                _("Cannot move" ) );

  }

  fputs ( "\n\n", pf );

  return;

}





/*
 * Print cube analysis
 *
 * Input:
 *  pf: output file
 *  pms: match state
 *  pmr: current move record
 *
 */

static void
TextAnalysis ( FILE *pf, matchstate *pms, moverecord *pmr ) {


  char sz[ 1024 ];

  switch ( pmr->mt ) {

  case MOVE_NORMAL:

    if ( pmr->n.anMove[ 0 ] >= 0 )
      fprintf ( pf,
                _("* %s moves %s"),
                ap[ pmr->n.fPlayer ].szName,
                FormatMove ( sz, pms->anBoard, pmr->n.anMove ) );
    else if ( ! pmr->n.ml.cMoves )
      fprintf ( pf,
                _("* %s cannot move"),
                ap[ pmr->n.fPlayer ].szName );

    fputs ( "\n", pf );

    TextPrintCubeAnalysis ( pf, pms, pmr );

    TextPrintMoveAnalysis ( pf, pms, pmr );

    break;

  case MOVE_DOUBLE:
  case MOVE_TAKE:
  case MOVE_DROP:

    if ( pmr->mt == MOVE_DOUBLE ) 
      fprintf ( pf,
                "* %s doubles\n\n",
                ap[ pmr->d.fPlayer ].szName );
    else
      fprintf ( pf,
                "* %s %s\n\n",
                ap[ pmr->d.fPlayer ].szName,
                ( pmr->mt == MOVE_TAKE ) ? _("accepts") : _("rejects") );
    
    TextPrintCubeAnalysis ( pf, pms, pmr );

    break;

  default:
    break;

  }

}


static void TextDumpStatcontext ( FILE *pf, const statcontext *psc,
                                  matchstate *pms, const int iGame ) {

  char sz[ 4096 ];

  if ( iGame >= 0 ) {
    fprintf ( pf, _("Game statistics for game %d\n\n"), iGame + 1 );
  }
  else {
    
    if ( pms->nMatchTo )
      fprintf ( pf, _( "Match statistics\n\n" ) );
    else
      fprintf ( pf, _( "Session statistics\n\n" ) );

  }

  DumpStatcontext ( sz, psc, NULL );

  fputs ( sz, pf );

  fputs ( "\n\n", pf );
}



static void
TextPrintComment ( FILE *pf, const moverecord *pmr ) {

  char *sz = NULL;

  switch ( pmr->mt ) {

  case MOVE_GAMEINFO:
    sz = pmr->g.sz;
    break;
  case MOVE_DOUBLE:
  case MOVE_TAKE:
  case MOVE_DROP:
    sz = pmr->d.sz;
    break;
  case MOVE_NORMAL:
    sz = pmr->n.sz;
    break;
  case MOVE_RESIGN:
    sz = pmr->r.sz;
    break;
  case MOVE_SETBOARD:
    sz = pmr->sb.sz;
    break;
  case MOVE_SETDICE:
    sz = pmr->sd.sz;
    break;
  case MOVE_SETCUBEVAL:
    sz = pmr->scv.sz;
    break;
  case MOVE_SETCUBEPOS:
    sz = pmr->scp.sz;
    break;

  }

  if ( sz ) {

    fputs ( _("Annotation:\n"), pf );
    fputs ( sz, pf );
    fputs ( "\n", pf );

  }


}

static void
TextMatchInfo ( FILE *pf, const matchinfo *pmi ) {

  int i;
  char sz[ 80 ];
  struct tm tmx;

  fputs ( _("Match Information:\n\n"), pf );

  /* ratings */

  for ( i = 0; i < 2; ++i )
      fprintf ( pf, _("%s's rating: %s\n"), 
                ap[ i ].szName, 
                pmi->pchRating[ i ] ? pmi->pchRating[ i ] : _("n/a") );

  /* date */

  if ( pmi->nYear ) {

    tmx.tm_year = pmi->nYear - 1900;
    tmx.tm_mon = pmi->nMonth - 1;
    tmx.tm_mday = pmi->nDay;
    strftime ( sz, sizeof ( sz ), _("%B %d, %Y"), &tmx );
    fprintf ( pf, _("Date: %s\n"), sz ); 

  }
  else
    fputs ( _("Date: n/a\n"), pf );

  /* event, round, place and annotator */

  fprintf ( pf, _("Event: %s\n"),
            pmi->pchEvent ? pmi->pchEvent : _("n/a") );
  fprintf ( pf, _("Round: %s\n"),
            pmi->pchRound ? pmi->pchRound : _("n/a") );
  fprintf ( pf, _("Place: %s\n"),
            pmi->pchPlace ? pmi->pchPlace : _("n/a") );
  fprintf ( pf, _("Annotator: %s\n"),
            pmi->pchAnnotator ? pmi->pchAnnotator : _("n/a") );
  fprintf ( pf, _("Comments: %s\n"),
            pmi->pchComment ? pmi->pchComment : _("n/a") );

}



static void
TextDumpPlayerRecords ( FILE *pf ) {

  /* dump the player records from file */
 
  playerrecord apr[ 2 ];
  playerrecord *pr;
  int i;
  int f = FALSE;
  int af[ 2 ] = { FALSE, FALSE };
  
  for ( i = 0; i < 2; ++i ) 
    if ( ( pr = GetPlayerRecord ( ap[ i ].szName ) ) ) {
      f = TRUE;
      af[ i ] = TRUE;
      memcpy ( &apr[ i ], pr, sizeof ( playerrecord ) );
    }

  if ( ! f )
    return;

  fputs ( _("Statistics from player records:\n\n" ), pf );
  
  fputs ( _("                                Short-term  "
            "Long-term   Total        Total\n"
            "                                error rate  "
            "error rate  error rate   luck\n"
            "Name                            Cheq. Cube  "
            "Cheq. Cube  Cheq. Cube   rate Games\n"), pf );

  for ( i = 0; i < 2; ++i ) 
    if ( af[ i ] ) 
      fprintf( pf, 
               "%-31s %5.3f %5.3f %5.3f %5.3f %5.3f %5.3f %6.3f %4d\n",
               apr[ i ].szName, apr[ i ].arErrorChequerplay[ EXPAVG_20 ],
               apr[ i ].arErrorCube[ EXPAVG_20 ],
               apr[ i ].arErrorChequerplay[ EXPAVG_100 ],
               apr[ i ].arErrorCube[ EXPAVG_100 ],
               apr[ i ].arErrorChequerplay[ EXPAVG_TOTAL ],
               apr[ i ].arErrorCube[ EXPAVG_TOTAL ],
               apr[ i ].arLuck[ EXPAVG_TOTAL ], apr[ i ].cGames );

  fputs ( "\n", pf );


}


/*
 * Export a game in HTML
 *
 * Input:
 *   pf: output file
 *   plGame: list of moverecords for the current game
 *
 */

static void ExportGameText ( FILE *pf, list *plGame, 
                             const int iGame, const int fLastGame ) {

    list *pl;
    moverecord *pmr;
    matchstate msExport;
    matchstate msOrig;
    int iMove = 0;
    statcontext *psc = NULL;
    static statcontext scTotal;
    movegameinfo *pmgi = NULL;

    if ( ! iGame )
      IniStatcontext ( &scTotal );

    updateStatisticsGame ( plGame );

    for( pl = plGame->plNext; pl != plGame; pl = pl->plNext ) {

      pmr = pl->p;

      FixMatchState ( &msExport, pmr );

      switch( pmr->mt ) {

      case MOVE_GAMEINFO:

        ApplyMoveRecord ( &msExport, plGame, pmr );

        TextPrologue( pf, &msExport, iGame );

        if ( exsExport.fIncludeMatchInfo )
          TextMatchInfo ( pf, &mi );

        msOrig = msExport;
        pmgi = &pmr->g;

        psc = &pmr->g.sc;

        AddStatcontext ( psc, &scTotal );
    
        /* FIXME: game introduction */
        break;

      case MOVE_NORMAL:

	if( pmr->n.fPlayer != msExport.fMove ) {
	    SwapSides( msExport.anBoard );
	    msExport.fMove = pmr->n.fPlayer;
	}
      
        msExport.fTurn = msExport.fMove = pmr->n.fPlayer;
        msExport.anDice[ 0 ] = pmr->n.anRoll[ 0 ];
        msExport.anDice[ 1 ] = pmr->n.anRoll[ 1 ];

        TextBoardHeader ( pf, &msExport, iGame, iMove );

        printTextBoard( pf, &msExport );
        TextAnalysis ( pf, &msExport, pmr );
        
        iMove++;

        break;

      case MOVE_TAKE:
      case MOVE_DROP:

        TextBoardHeader ( pf,&msExport, iGame, iMove );

        printTextBoard( pf, &msExport );
        
        TextAnalysis ( pf, &msExport, pmr );
        
        iMove++;

        break;


      default:

        break;
        
      }

      TextPrintComment ( pf, pmr );

      ApplyMoveRecord ( &msExport, plGame, pmr );

    }

    if( pmgi && pmgi->fWinner != -1 ) {

      /* print game result */

      if ( pmgi->nPoints > 1 )
        fprintf ( pf, 
                  _("%s wins %d points\n\n"),
                  ap[ pmgi->fWinner ].szName, 
                  pmgi->nPoints );
      else
        fprintf ( pf, 
                  _("%s wins %d point\n\n"),
                  ap[ pmgi->fWinner ].szName,
                  pmgi->nPoints );

    }

    if ( psc ) {
      TextDumpStatcontext ( pf, psc, &msOrig, iGame );
      TextDumpPlayerRecords ( pf );
    }


    if ( fLastGame ) {

      TextDumpStatcontext ( pf, &scTotal, &msOrig, -1 );

    }

    TextEpilogue( pf, &msExport );


    
}

extern void CommandExportGameText( char *sz ) {

    FILE *pf;
    
    sz = NextToken( &sz );
    
    if( !plGame ) {
	outputl( _("No game in progress (type `new game' to start one).") );
	return;
    }
    
    if( !sz || !*sz ) {
	outputl( _("You must specify a file to export to (see `help export"
		 "game text').") );
	return;
    }

    if ( ! confirmOverwrite ( sz, fConfirmSave ) )
      return;

    if( !strcmp( sz, "-" ) )
	pf = stdout;
    else if( !( pf = fopen( sz, "w" ) ) ) {
	outputerr( sz );
	return;
    }

    ExportGameText( pf, plGame,
                    getGameNumber ( plGame ), FALSE );

    if( pf != stdout )
	fclose( pf );

    setDefaultFileName ( sz, PATH_TEXT );
    
}




extern void CommandExportMatchText( char *sz ) {
    
    FILE *pf;
    list *pl;
    int nGames;
    char *szCurrent;
    int i;

    sz = NextToken( &sz );
    
    if( !sz || !*sz ) {
	outputl( _("You must specify a file to export to (see `help export "
		 "match text').") );
	return;
    }

    /* Find number of games in match */

    for( pl = lMatch.plNext, nGames = 0; pl != &lMatch; 
         pl = pl->plNext, nGames++ )
      ;

    for( pl = lMatch.plNext, i = 0; pl != &lMatch; pl = pl->plNext, i++ ) {

      szCurrent = HTMLFilename ( sz, i );

      if ( !i ) {

        if ( ! confirmOverwrite ( sz, fConfirmSave ) )
          return;

        setDefaultFileName ( sz, PATH_TEXT );

      }


      if( !strcmp( szCurrent, "-" ) )
	pf = stdout;
      else if( !( pf = fopen( szCurrent, "w" ) ) ) {
	outputerr( szCurrent );
	return;
      }

      ExportGameText ( pf, pl->p, 
                       i, i == nGames - 1 );

      if( pf != stdout )
        fclose( pf );

    }
    
}



extern void CommandExportPositionText( char *sz ) {

    FILE *pf;
    int fHistory;
    moverecord *pmr = getCurrentMoveRecord ( &fHistory );
    int iMove;
	
    sz = NextToken( &sz );
    
    if( ms.gs == GAME_NONE ) {
	outputl( _("No game in progress (type `new game' to start one).") );
	return;
    }
    
    if( !sz || !*sz ) {
	outputl( _("You must specify a file to export to (see `help export "
		 "position text').") );
	return;
    }


    if ( ! confirmOverwrite ( sz, fConfirmSave ) )
      return;

    if( !strcmp( sz, "-" ) )
	pf = stdout;
    else if( !( pf = fopen( sz, "w" ) ) ) {
	outputerr( sz );
	return;
    }

    TextPrologue ( pf, &ms, getGameNumber ( plGame ) );

    if ( exsExport.fIncludeMatchInfo )
      TextMatchInfo ( pf, &mi );

    if ( fHistory )
      iMove = getMoveNumber ( plGame, pmr ) - 1;
    else if ( plLastMove )
      iMove = getMoveNumber ( plGame, plLastMove->p );
    else
      iMove = -1;

    TextBoardHeader ( pf, &ms, 
                      getGameNumber ( plGame ), iMove );

    printTextBoard( pf, &ms );

    if( pmr ) {

      TextAnalysis ( pf, &ms, pmr );

      TextPrintComment ( pf, pmr );

    }
    
    TextEpilogue ( pf, &ms );

    if( pf != stdout )
	fclose( pf );

    setDefaultFileName ( sz, PATH_TEXT );

}

