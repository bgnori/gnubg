/*
 * show.c
 *
 * by Gary Wong <gtw@gnu.org>, 1999, 2000, 2001, 2002.
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
 * $Id: show.c,v 1.94 2002/06/06 20:46:48 gtw Exp $
 */

#include "config.h"

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <assert.h>

#include "backgammon.h"
#include "drawboard.h"
#include "eval.h"
#include "export.h"
#include "dice.h"
#include "matchequity.h"
#include "matchid.h"
#include "i18n.h"

#if USE_GTK
#include "gtkboard.h"
#include "gtkgame.h"
#include "gtktheory.h"
#include "gtkexport.h"
#elif USE_EXT
#include "xgame.h"
#endif

extern char *aszCopying[], *aszWarranty[]; /* from copying.c */

static void ShowEvaluation( evalcontext *pec ) {
    
    outputf( _("        %d-ply evaluation.\n"
             "        %d move search candidate%s.\n"
             "        %0.3g cubeless search tolerance.\n"
             "        %d%% speed.\n"
             "        %s pruning at 1-ply for moves.\n"
             "        %s evaluations.\n"),
             pec->nPlies, pec->nSearchCandidates, pec->nSearchCandidates == 1 ?
             "" : "s", pec->rSearchTolerance,
             (pec->nReduced) ? 100 / pec->nReduced : 100,
             pec->fNoOnePlyPrune ? _("No") : _("Normal"),
             pec->fCubeful ? _("Cubeful") : _("Cubeless") );

    if( pec->rNoise )
	outputf( _("    Noise standard deviation %5.3f"), pec->rNoise );
    else
	output( _("    Noiseless evaluations") );

    outputl( pec->fDeterministic ? _(" (deterministic noise).\n") :
	     _(" (pseudo-random noise).\n") );
}


extern void
ShowRollout ( rolloutcontext *prc ) {

  int i;

  if ( prc->nTrials == 1 )
    outputf ( _("%d game will be played per rollout.\n"),
              prc->nTrials );
  else
    outputf ( _("%d games will be played per rollout.\n"),
              prc->nTrials );

  if( prc->nTruncate > 1 )
      outputf( _("Truncation after %d plies.\n"), prc->nTruncate );
  else if( prc->nTruncate == 1 )
      outputl( _("Truncation after 1 ply.") );
  else
      outputl( _("No truncation.") );

  outputl ( prc->fVarRedn ? 
            _("Lookahead variance reduction is enabled.") :
            _("Lookahead variance reduction is disabled.") );
  outputl ( prc->fCubeful ?
            _("Cubeful rollout.") :
            _("Cubeless rollout.") );
  outputl ( prc->fInitial ?
            _("Rollout as opening move enabled.") :
            _("Rollout as opening move disabled.") );
  outputf ( _("%s dice generator with seed %u.\n"),
            gettext ( aszRNG[ prc->rngRollout ] ), prc->nSeed );
            
  /* FIXME: more compact notation when aecCube = aecChequer etc. */

  outputl (_("Chequer play parameters:"));

  for ( i = 0; i < 2; i++ ) {
    outputf ( _("  Player %d:\n"), i );
    ShowEvaluation ( &prc->aecChequer[ i ] );
  }

  outputl (_("Cube decision parameters:"));

  for ( i = 0; i < 2; i++ ) {
    outputf ( _("  Player %d:\n"), i );
    ShowEvaluation ( &prc->aecCube[ i ] );
  }

}


extern void
ShowEvalSetup ( evalsetup *pes ) {

  switch ( pes->et ) {

  case EVAL_NONE:
    outputl ( _("      No evaluation.") );
    break;
  case EVAL_EVAL:
    outputl ( _("      Neural net evaluation:") );
    ShowEvaluation ( &pes->ec );
    break;
  case EVAL_ROLLOUT:
    outputl ( _("      Rollout:") );
    ShowRollout ( &pes->rc );
    break;
  default:
    assert ( FALSE );

  }

}


static void ShowPaged( char **ppch ) {

    int i, nRows = 0;
    char *pchLines;
#ifdef TIOCGWINSZ
    struct winsize ws;
#endif

#if HAVE_ISATTY
    if( isatty( STDIN_FILENO ) ) {
#endif
#ifdef TIOCGWINSZ
	if( !( ioctl( STDIN_FILENO, TIOCGWINSZ, &ws ) ) )
	    nRows = ws.ws_row;
#endif
	if( !nRows && ( pchLines = getenv( "LINES" ) ) )
	    nRows = atoi( pchLines );

	/* FIXME we could try termcap-style tgetnum( "li" ) here, but it
	   hardly seems worth it */
	
	if( !nRows )
	    nRows = 24;

	i = 0;

	while( *ppch ) {
	    outputl( *ppch++ );
	    if( ++i >= nRows - 1 ) {
		GetInput( _("-- Press <return> to continue --") );
		
		if( fInterrupt )
		    return;
		
		i = 0;
	    }
	}
#if HAVE_ISATTY
    } else
#endif
	while( *ppch )
	    outputl( *ppch++ );
}

extern void CommandShowAnalysis( char *sz ) {

    outputl( fAnalyseCube ? _("Cube action will be analysed.") :
	     _("Cube action will not be analysed.") );

    outputl( fAnalyseDice ? _("Dice rolls will be analysed.") :
	     _("Dice rolls will not be analysed.") );

    if( fAnalyseMove ) {
	outputl( _("Chequer play will be analysed.") );
	if( cAnalysisMoves < 0 )
	    outputl( _("Every legal move will be analysed.") );
	else
	    outputf( _("Up to %d moves will be analysed.\n"), cAnalysisMoves );
    } else
	outputl( _("Chequer play will not be analysed.") );

    outputl( _("\nAnalysis thresholds:") );
    outputf( "  +%.3f %s\n"
	     "  +%.3f %s\n"
	     "  -%.3f %s\n"
	     "  -%.3f %s\n"
	     "  -%.3f %s\n"
	     "\n"
	     "  +%.3f %s\n"
	     "  +%.3f %s\n"
	     "  -%.3f %s\n"
	     "  -%.3f %s\n",
	     arSkillLevel[ SKILL_VERYGOOD ], 
             gettext ( aszSkillType[ SKILL_VERYGOOD ] ),
             arSkillLevel[ SKILL_GOOD ], 
             gettext ( aszSkillType[ SKILL_GOOD ] ),
	     arSkillLevel[ SKILL_DOUBTFUL ], 
             gettext ( aszSkillType[ SKILL_DOUBTFUL ] ),
	     arSkillLevel[ SKILL_BAD ], 
             gettext ( aszSkillType[ SKILL_BAD ] ),
             arSkillLevel[ SKILL_VERYBAD ],
             gettext ( aszSkillType[ SKILL_VERYBAD ] ),
	     arLuckLevel[ LUCK_VERYGOOD ], 
             gettext ( aszLuckType[ LUCK_VERYGOOD ] ),
             arLuckLevel[ LUCK_GOOD ],
             gettext ( aszLuckType[ LUCK_GOOD ] ),
	     arLuckLevel[ LUCK_BAD ], 
             gettext ( aszLuckType[ LUCK_BAD ] ),
             arLuckLevel[ LUCK_VERYBAD ],
             gettext ( aszLuckType[ LUCK_VERYBAD ] ) );

    outputl( _("\n"
               "Analysis will be performed with the "
             "following evaluation parameters:") );
  outputl( _("    Chequer play:") );
    ShowEvalSetup ( &esAnalysisChequer );
    outputl( _("    Cube decisions:") );
    ShowEvalSetup ( &esAnalysisCube );

    

}

extern void CommandShowAutomatic( char *sz ) {

    static char *szOn = N_("On"), *szOff = N_("Off");
    
    outputf( 
            _("analysis \t(Analyse game during play (tutor-mode)):      \t%s\n"
              "bearoff \t(Play certain non-contact bearoff moves):      \t%s\n"
              "crawford\t(Enable the Crawford rule as appropriate):     \t%s\n"
              "doubles \t(Turn the cube when opening roll is a double): \t%d\n"
              "game    \t(Start a new game after each one is completed):\t%s\n"
              "move    \t(Play the forced move when there is no choice):\t%s\n"
              "roll    \t(Roll the dice if no double is possible):      \t%s\n"),
	    fAutoAnalysis ? gettext ( szOn ) : gettext ( szOff ),
	    fAutoBearoff ? gettext ( szOn ) : gettext ( szOff ),
	    fAutoCrawford ? gettext ( szOn ) : gettext ( szOff ),
	    cAutoDoubles,
	    fAutoGame ? gettext ( szOn ) : gettext ( szOff ),
	    fAutoMove ? gettext ( szOn ) : gettext ( szOff ),
	    fAutoRoll ? gettext ( szOn ) : gettext ( szOff ) );
}

extern void CommandShowBoard( char *sz ) {

    int an[ 2 ][ 25 ];
    char szOut[ 2048 ];
    char *ap[ 7 ] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    
    if( !*sz ) {
	if( ms.gs == GAME_NONE )
	    outputl( _("No position specified and no game in progress.") );
	else
	    ShowBoard();
	
	return;
    }

    /* FIXME handle =n notation */
    if( ParsePosition( an, &sz, NULL ) < 0 )
	return;

#if USE_GUI
    if( fX )
#if USE_GTK
	game_set( BOARD( pwBoard ), an, TRUE, "", "", 0, 0, 0, -1, -1, FALSE );
#else
        GameSet( &ewnd, an, TRUE, "", "", 0, 0, 0, -1, -1 );    
#endif
    else
#endif
        outputl( DrawBoard( szOut, an, TRUE, ap, 
                            MatchIDFromMatchState ( &ms ) ) );
}

extern void CommandShowDelay( char *sz ) {
#if USE_GUI
    if( nDelay )
	outputf( _("The delay is set to %d ms.\n"),nDelay);
    else
	outputl( _("No delay is being used.") );
#else
    outputl( _("The `show delay' command applies only when using a window "
	  "system.") );
#endif
}

extern void CommandShowCache( char *sz ) {

    int c, cLookup, cHit;

    EvalCacheStats( &c, NULL, &cLookup, &cHit );

    outputf( _("%d cache entries have been used.  %d lookups, %d hits"),
	    c, cLookup, cHit );

    if( cLookup > 0x01000000 ) /* calculate carefully to avoid overflow */
	outputf( " (%d%%).", ( cHit + ( cLookup / 200 ) ) /
		 ( cLookup / 100 ) );
    else if( cLookup )
	outputf( " (%d%%).", ( cHit * 100 + cLookup / 2 ) / cLookup );
    else
	outputc( '.' );

    outputc( '\n' );
}

extern void CommandShowClockwise( char *sz ) {

    if( fClockwise )
	outputl( _("Player 1 moves clockwise (and player 0 moves "
		 "anticlockwise).") );
    else
	outputl( _("Player 1 moves anticlockwise (and player 0 moves "
		 "clockwise).") );
}

static void ShowCommands( command *pc, char *szPrefix ) {

    char sz[ 128 ], *pch;

    strcpy( sz, szPrefix );
    pch = strchr( sz, 0 );

    for( ; pc->sz; pc++ ) {
	if( !pc->szHelp )
	    continue;

	strcpy( pch, pc->sz );

	if( pc->pc && pc->pc->pc != pc->pc ) {
	    strcat( pch, " " );
	    ShowCommands( pc->pc, sz );
	} else
	    outputl( sz );
    }
}

extern void CommandShowCommands( char *sz ) {

    ShowCommands( acTop, "" );
}

extern void CommandShowConfirm( char *sz ) {

    if( fConfirm )
	outputl( _("GNU Backgammon will ask for confirmation before "
	       "aborting games in progress.") );
    else
	outputl( _("GNU Backgammon will not ask for confirmation "
	       "before aborting games in progress.") );

    if( fConfirmSave )
	outputl( _("GNU Backgammon will ask for confirmation before "
	       "overwriting existing files.") );
    else
	outputl( _("GNU Backgammon will not ask for confirmation "
	       "overwriting existing files.") );

}

extern void CommandShowCopying( char *sz ) {

#if USE_GTK
    if( fX )
	ShowList( aszCopying, _("Copying") );
    else
#endif
	ShowPaged( aszCopying );
}

extern void CommandShowCrawford( char *sz ) {

  if( ms.nMatchTo > 0 ) 
    outputl( ms.fCrawford ?
	  _("This game is the Crawford game.") :
	  _("This game is not the Crawford game") );
  else if ( !ms.nMatchTo )
    outputl( _("Crawford rule is not used in money sessions.") );
  else
    outputl( _("No match is being played.") );

}

extern void CommandShowCube( char *sz ) {

    if( ms.gs != GAME_PLAYING ) {
	outputl( _("There is no game in progress.") );
	return;
    }

    if( ms.fCrawford ) {
	outputl( _("The cube is disabled during the Crawford game.") );
	return;
    }
    
    if( !fCubeUse ) {
	outputl( _("The doubling cube is disabled.") );
	return;
    }
	
    if( ms.fCubeOwner == -1 )
	outputf( _("The cube is at %d, and is centred."), ms.nCube );
    else
	outputf( _("The cube is at %d, and is owned by %s."), 
                 ms.nCube, ap[ ms.fCubeOwner ].szName );
}

extern void CommandShowDice( char *sz ) {

    if( ms.gs != GAME_PLAYING ) {
	outputl( _("The dice will not be rolled until a game is started.") );

	return;
    }

    if( ms.anDice[ 0 ] < 1 )
	outputf( _("%s has not yet rolled the dice.\n"), ap[ ms.fMove ].szName );
    else
	outputf( _("%s has rolled %d and %d.\n"), ap[ ms.fMove ].szName,
		 ms.anDice[ 0 ], ms.anDice[ 1 ] );
}

extern void CommandShowDisplay( char *sz ) {

    if( fDisplay )
	outputl( _("GNU Backgammon will display boards for computer moves.") );
    else
	outputl( _("GNU Backgammon will not display boards for computer moves.") );
}

extern void CommandShowEngine( char *sz ) {

    char szBuffer[ 4096 ];
    
    EvalStatus( szBuffer );

    output( szBuffer );
}

extern void CommandShowEvaluation( char *sz ) {

    outputl( _("`eval' and `hint' will use:") );
    outputl( _("    Chequer play:") );
    ShowEvalSetup ( &esEvalChequer );
    outputl( _("    Cube decisions:") );
    ShowEvalSetup ( &esEvalCube );

}

extern void CommandShowEgyptian( char *sz ) {

    if ( fEgyptian )
      outputl( _("Sessions are played with the Egyptian rule.") );
    else
      outputl( _("Sessions are played without the Egyptian rule.") );

}

extern void CommandShowJacoby( char *sz ) {

    if ( fJacoby ) 
      outputl( _("Money sessions are played with the Jacoby rule.") );
    else
      outputl( _("Money sessions are played without the Jacoby rule.") );

}

extern void CommandShowNackgammon( char *sz ) {

    if( fNackgammon )
	outputl( _("New games will use the Nackgammon starting position.") );
    else
	outputl( _("New games will use the standard backgammon starting "
	      "position.") );
}

extern void CommandShowPipCount( char *sz ) {

    int anPips[ 2 ], an[ 2 ][ 25 ];

    if( !*sz && ms.gs == GAME_NONE ) {
	outputl( _("No position specified and no game in progress.") );
	return;
    }
    
    if( ParsePosition( an, &sz, NULL ) < 0 )
	return;
    
    PipCount( an, anPips );
    
    outputf( _("The pip counts are: %s %d, %s %d.\n"), ap[ ms.fMove ].szName,
	    anPips[ 1 ], ap[ !ms.fMove ].szName, anPips[ 0 ] );
}

extern void CommandShowPlayer( char *sz ) {

    int i;

    for( i = 0; i < 2; i++ ) {
	outputf( _("Player %d:\n"
		"  Name: %s\n"
		"  Type: "), i, ap[ i ].szName );

	switch( ap[ i ].pt ) {
	case PLAYER_EXTERNAL:
	    outputf( _("external: %s\n\n"), ap[ i ].szSocket );
	    break;
	case PLAYER_GNU:
	    outputf( _("gnubg:\n") );
            outputl( _("    Checker play:") );
            ShowEvalSetup ( &ap[ i ].esChequer );
            outputl( _("    Cube decisions:") );
            ShowEvalSetup ( &ap[ i ].esCube );
	    break;
	case PLAYER_PUBEVAL:
	    outputl( _("pubeval\n") );
	    break;
	case PLAYER_HUMAN:
	    outputl( _("human\n") );
	    break;
	}
    }
}

extern void CommandShowPostCrawford( char *sz ) {

  if( ms.nMatchTo > 0 ) 
    outputl( ms.fPostCrawford ?
	  _("This is post-Crawford play.") :
	  _("This is not post-Crawford play.") );
  else if ( !ms.nMatchTo )
    outputl( _("Crawford rule is not used in money sessions.") );
  else
    outputl( _("No match is being played.") );

}

extern void CommandShowPrompt( char *sz ) {

    outputf( _("The prompt is set to `%s'.\n"), szPrompt );
}

extern void CommandShowRNG( char *sz ) {

  outputf( _("You are using the %s generator.\n"),
	  gettext ( aszRNG[ rngCurrent ] ) );
    
}

extern void CommandShowRollout( char *sz ) {

  outputl( _("`rollout' will use:") );
  ShowRollout ( &rcRollout );

}

extern void CommandShowScore( char *sz ) {

    outputf( _("The score (after %d game%s) is: %s %d, %s %d"),
	    ms.cGames, ms.cGames == 1 ? "" : "s",
	    ap[ 0 ].szName, ms.anScore[ 0 ],
	    ap[ 1 ].szName, ms.anScore[ 1 ] );

    if ( ms.nMatchTo > 0 ) {
        outputf ( ms.nMatchTo == 1 ? 
	         _(" (match to %d point%s).\n") :
	         _(" (match to %d points%s).\n"),
                 ms.nMatchTo,
		 ms.fCrawford ? 
		 _(", Crawford game") : ( ms.fPostCrawford ?
					 _(", post-Crawford play") : ""));
    } 
    else {
        if ( fJacoby )
	    outputl ( _(" (money session,\nwith Jacoby rule).") );
        else
	    outputl ( _(" (money session,\nwithout Jacoby rule).") );
    }

}

extern void CommandShowSeed( char *sz ) {

    PrintRNGSeed();
}

extern void CommandShowTurn( char *sz ) {

    if( ms.gs != GAME_PLAYING ) {
	outputl( _("No game is being played.") );

	return;
    }
    
    if ( ms.anDice[ 0 ] )
      outputf ( _("%s in on move.\n"),
                ap[ ms.fMove ].szName );
    else
      outputf ( _("%s in on roll.\n"),
                ap[ ms.fMove ].szName );

    if( ms.fResigned )
	outputf( _("%s has offered to resign a %s.\n"), ap[ ms.fMove ].szName,
		gettext ( aszGameResult[ ms.fResigned - 1 ] ) );
}

extern void CommandShowWarranty( char *sz ) {

#if USE_GTK
    if( fX )
	ShowList( aszWarranty, _("Warranty") );
    else
#endif
	ShowPaged( aszWarranty );
}

extern void CommandShowKleinman( char *sz ) {

    int anPips[ 2 ], an[ 2 ][ 25 ];
    float fKC;

    if( !*sz && ms.gs == GAME_NONE ) {
        outputl( _("No position specified and no game in progress.") );
        return;
    }
 
    if( ParsePosition( an, &sz, NULL ) < 0 )
	return;
     
    PipCount( an, anPips );
 
    fKC = KleinmanCount (anPips[1], anPips[0]);
    if (fKC == -1.0)
        outputf (_("Pipcount unsuitable for Kleinman Count.\n"));
    else
        outputf (_("Cubeless Winning Chance: %.4f\n"), fKC);
 }

extern void CommandShowThorp( char *sz ) {

    int anPips[ 2 ], an[ 2 ][ 25 ];
    int nLeader, nTrailer, nDiff, anCovered[2], anMenLeft[2];
    int x;

    if( !*sz && ms.gs == GAME_NONE ) {
        outputl( _("No position specified and no game in progress.") );
        return;
    }

    if( ParsePosition( an, &sz, NULL ) < 0 )
	return;

    PipCount( an, anPips );

  anMenLeft[0] = 0;
  anMenLeft[1] = 0;
  for (x = 0; x < 25; x++)
    {
      anMenLeft[0] += an[0][x];
      anMenLeft[1] += an[1][x];
    }

  anCovered[0] = 0;
  anCovered[1] = 0;
  for (x = 0; x < 6; x++)
    {
      if (an[0][x])
        anCovered[0]++;
      if (an[1][x])
        anCovered[1]++;
    }

        nLeader = anPips[1];
        nLeader += 2*anMenLeft[1];
        nLeader += an[1][0];
        nLeader -= anCovered[1];

        if (nLeader > 30) {
         if ((nLeader % 10) > 5)
        {
           nLeader *= 1.1;
           nLeader += 1;
        }
         else
          nLeader *= 1.1;
        }
        nTrailer = anPips[0];
        nTrailer += 2*anMenLeft[0];
        nTrailer += an[0][0];
        nTrailer -= anCovered[0];

        outputf("L = %d  T = %d  -> ", nLeader, nTrailer);

        if (nTrailer >= (nLeader - 1))
          output(_("Redouble, "));
        else if (nTrailer >= (nLeader - 2))
          output(_("Double, "));
        else
          output(_("No double, "));

        if (nTrailer >= (nLeader + 2))
          outputl(_("drop"));
        else
          outputl(_("take"));

	if( ( nDiff = nTrailer - nLeader ) > 13 )
	    nDiff = 13;
	else if( nDiff < -37 )
	    nDiff = -37;
	
        outputf(_("Bower's interpolation: %d%% cubeless winning "
                "chance\n"), 74 + 2 * nDiff );
}

extern void CommandShowBeavers( char *sz ) {

    if( nBeavers > 1 )
	outputf( _("%d beavers/racoons allowed in money sessions.\n"), nBeavers );
    else if( nBeavers == 1 )
	outputl( _("1 beaver allowed in money sessions.") );
    else
	outputl( _("No beavers allowed in money sessions.") );
}

extern void CommandShowGammonValues ( char *sz ) {

  cubeinfo ci;
  int i;

  if( ms.gs != GAME_PLAYING ) {
    outputl( _("No game in progress (type `new game' to start one).") );

    return;
  }

#if USE_GTK
  if ( fX ) {
    GTKShowTheory ( 1 );
    return;
  }
#endif

  GetMatchStateCubeInfo( &ci, &ms );

  output ( _("Player        Gammon value    Backgammon value\n") );

  for ( i = 0; i < 2; i++ ) {

    outputf ("%-12s     %7.5f         %7.5f\n",
	     ap[ i ].szName,
	     ci.arGammonPrice[ i ], ci.arGammonPrice[ 2 + i ] );
  }

}

static void
writeMET ( float aafMET[][ MAXSCORE ],
           const int nRows, const int nCols, const int fInvert ) {

  int i,j;

  output ( "          " );
  for ( j = 0; j < nCols; j++ )
    outputf ( _(" %3i-away "), j + 1 );
  output ( "\n" );

  for ( i = 0; i < nRows; i++ ) {
    
    outputf ( _(" %3i-away "), i + 1 );
    
    for ( j = 0; j < nCols; j++ )
      outputf ( " %8.4f ", 
                fInvert ? 100.0f * ( 1.0 - GET_MET ( i, j, aafMET ) ) :
                GET_MET ( i, j, aafMET ) * 100.0 );
    output ( "\n" );
  }
  output ( "\n" );

}


extern void CommandShowMatchEquityTable ( char *sz ) {

  /* Read a number n. */

  int n = ParseNumber ( &sz );
  int i;

  /* If n > 0 write n x n match equity table,
     else if match write nMatchTo x nMatchTo table,
     else write full table (may be HUGE!) */

  if ( ( n <= 0 ) || ( n > MAXSCORE ) ) {
    if ( ms.nMatchTo )
      n = ms.nMatchTo;
    else
      n = MAXSCORE;
  }

#if USE_GTK
  if( fX ) {
      GTKShowMatchEquityTable( n );
      return;
  }
#endif

  output ( _("Match equity table: ") );
  outputl( miCurrent.szName );
  outputf( "(%s)\n", miCurrent.szFileName );
  outputl( miCurrent.szDescription );
  outputl( "" );
  
  /* write tables */

  output ( _("Pre-Crawford table:\n\n") );
  writeMET ( aafMET, n, n, FALSE );

  for ( i = 0; i < 2; i++ ) {
    outputf ( _("Post-Crawford table for player %d (%s):\n\n"),
              i, ap[ i ].szName );
  writeMET ( (float (*)[MAXSCORE] ) aafMETPostCrawford[ i ], 1, n, TRUE );
  }
  
}

extern void CommandShowOutput( char *sz ) {

    outputf( fOutputMatchPC ? 
             _("Match winning chances will be shown as percentages.\n") :
             _("Match winning chances will be shown as probabilities.\n") );

    if ( fOutputMWC )
	outputl( _("Match equities shown in MWC (match winning chance) "
	      "(match play only).") ); 
    else
	outputl( _("Match equities shown in EMG (normalized money game equity) "
	      "(match play only).") ); 

    outputf( fOutputWinPC ? 
             _("Game winning chances will be shown as percentages.\n") :
             _("Game winning chances will be shown as probabilities.\n") );

#if USE_GUI
    if( !fX )
#endif
      outputf( fOutputRawboard ? 
               _("Boards will be shown in raw format.\n") :
               _("Boards will be shown in ASCII.\n") );
}

extern void CommandShowTraining( char *sz ) {

    outputf( _("Learning rate (alpha) %f,\n"), rAlpha );

    if( rAnneal )
	outputf( _("Annealing rate %f,\n"), rAnneal );
    else
	outputl( _("Annealing disabled,") );

    if( rThreshold )
	outputf( _("Error threshold %f.\n"), rThreshold );
    else
	outputl( _("Error threshold disabled.") );
}

extern void CommandShowVersion( char *sz ) {

    char **ppch = aszVersion;
    extern char *aszCredits[];
    int i = 0, cch, cCol = 0;
    
#if USE_GTK
    if( fX ) {
	GTKShowVersion();
	return;
    }
#endif

    while( *ppch )
	outputl( gettext ( *ppch++ ) );

    outputc( '\n' );

    outputl( _("GNU Backgammon was written by Joseph Heled, �ystein Johansen, "
	     "David Montgomery,\nJ�rn Thyssen and Gary Wong.\n\n"
	     "Special thanks to:") );

    cCol = 80;

    for( ppch = aszCredits; *ppch; ppch++ ) {
	i += ( cch = strlen( *ppch ) + 2 );
	if( i >= cCol ) {
	    outputc( '\n' );
	    i = cch;
	}
	
	outputf( "%s%c ", *ppch, *( ppch + 1 ) ? ',' : '.' );
    }

    outputc( '\n' );
}

extern void CommandShowMarketWindow ( char * sz ) {

  cubeinfo ci;

  float arOutput[ NUM_OUTPUTS ];
  float arDP1[ 2 ], arDP2[ 2 ],arCP1[ 2 ], arCP2[ 2 ];
  float rDTW, rDTL, rNDW, rNDL, rDP, rRisk, rGain, r;

  float aarRates[ 2 ][ 2 ];

  int i, fAutoRedouble[ 2 ], afDead[ 2 ], anNormScore[ 2 ];

  if( ms.gs != GAME_PLAYING ) {
    outputl( _("No game in progress (type `new game' to start one).") );

    return;
  }
      
#if USE_GTK
    if( fX ) {
      GTKShowTheory( 0 ); 
      return;
    }
#endif


  /* Show market window */

  /* First, get gammon and backgammon percentages */
  GetMatchStateCubeInfo( &ci, &ms );

  /* see if ratios are given on command line */

  aarRates[ 0 ][ 0 ] = ParseReal ( &sz );

  if ( aarRates[ 0 ][ 0 ] >= 0 ) {

    /* read the others */

    aarRates[ 1 ][ 0 ]  = ( (r = ParseReal ( &sz ) ) > 0.0) ? r : 0.0;
    aarRates[ 0 ][ 1 ] = ( (r = ParseReal ( &sz ) ) > 0.0) ? r : 0.0;
    aarRates[ 1 ][ 1 ] = ( (r = ParseReal ( &sz ) ) > 0.0) ? r : 0.0;

    /* If one of the ratios are larger than 1 we assume the user
       has entered 25.1 instead of 0.251 */

    if ( aarRates[ 0 ][ 0 ] > 1.0 || aarRates[ 1 ][ 0 ] > 1.0 ||
         aarRates[ 1 ][ 1 ] > 1.0 || aarRates[ 1 ][ 1 ] > 1.0 ) {
      aarRates[ 0 ][ 0 ]  /= 100.0;
      aarRates[ 1 ][ 0 ]  /= 100.0;
      aarRates[ 0 ][ 1 ] /= 100.0;
      aarRates[ 1 ][ 1 ] /= 100.0;
    }

    /* Check that ratios are 0 <= ratio <= 1 */

    for ( i = 0; i < 2; i++ ) {
      if ( aarRates[ i ][ 0 ] > 1.0 ) {
        outputf ( _("illegal gammon ratio for player %i: %f\n"),
                  i, aarRates[ i ][ 0 ] );
        return ;
      }
      if ( aarRates[ i ][ 1 ] > 1.0 ) {
        outputf ( _("illegal backgammon ratio for player %i: %f\n"),
                  i, aarRates[ i ][ 1 ] );
        return ;
      }
    }

    /* Transfer rations to arOutput
       (used in call to GetPoints below)*/ 

    arOutput[ OUTPUT_WIN ] = 0.5;
    arOutput[ OUTPUT_WINGAMMON ] =
      ( aarRates[ ms.fMove ][ 0 ] + aarRates[ ms.fMove ][ 1 ] ) * 0.5;
    arOutput[ OUTPUT_LOSEGAMMON ] =
      ( aarRates[ !ms.fMove ][ 0 ] + aarRates[ !ms.fMove ][ 1 ] ) * 0.5;
    arOutput[ OUTPUT_WINBACKGAMMON ] = aarRates[ ms.fMove ][ 1 ] * 0.5;
    arOutput[ OUTPUT_LOSEBACKGAMMON ] = aarRates[ !ms.fMove ][ 1 ] * 0.5;

  } else {

    /* calculate them based on current position */

    if ( getCurrentGammonRates ( aarRates, arOutput, ms.anBoard, 
                                 &ci, &esEvalCube.ec ) < 0 ) 
      return;

  }



  for ( i = 0; i < 2; i++ ) 
    outputf ( _("Player %-25s: gammon rate %6.2f%%, bg rate %6.2f%%\n"),
              ap[ i ].szName, 
              aarRates[ i ][ 0 ] * 100.0, aarRates[ i ][ 1 ] * 100.0);


  if ( ms.nMatchTo ) {

    for ( i = 0; i < 2; i++ )
      anNormScore[ i ] = ms.nMatchTo - ms.anScore[ i ];

    GetPoints ( arOutput, &ci, arCP2 );

    for ( i = 0; i < 2; i++ ) {

      fAutoRedouble [ i ] =
        ( anNormScore[ i ] - 2 * ms.nCube <= 0 ) &&
        ( anNormScore[ ! i ] - 2 * ms.nCube > 0 );

      afDead[ i ] =
        ( anNormScore[ ! i ] - 2 * ms.nCube <=0 );

      /* MWC for "double, take; win" */

      rDTW =
        (1.0 - aarRates[ i ][ 0 ] - aarRates[ i ][ 1 ]) *
        getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                2 * ms.nCube, i, ms.fCrawford,
                aafMET, aafMETPostCrawford )
        + aarRates[ i ][ 0 ] * 
        getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                4 * ms.nCube, i, ms.fCrawford, 
                aafMET, aafMETPostCrawford )
        + aarRates[ i ][ 1 ] * 
        getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                6 * ms.nCube, i, ms.fCrawford, 
                aafMET, aafMETPostCrawford );

      /* MWC for "no double, take; win" */

      rNDW =
        (1.0 - aarRates[ i ][ 0 ] - aarRates[ i ][ 1 ]) *
        getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                ms.nCube, i, ms.fCrawford,
                aafMET, aafMETPostCrawford )
        + aarRates[ i ][ 0 ] * 
        getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                2 * ms.nCube, i, ms.fCrawford, 
                aafMET, aafMETPostCrawford )
        + aarRates[ i ][ 1 ] * 
        getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                3 * ms.nCube, i, ms.fCrawford, 
                aafMET, aafMETPostCrawford );

      /* MWC for "Double, take; lose" */

      rDTL =
        (1.0 - aarRates[ ! i ][ 0 ] - aarRates[ ! i ][ 1 ]) *
        getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                2 * ms.nCube, ! i, ms.fCrawford,
                aafMET, aafMETPostCrawford )
        + aarRates[ ! i ][ 0 ] * 
        getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                4 * ms.nCube, ! i, ms.fCrawford, 
                aafMET, aafMETPostCrawford )
        + aarRates[ ! i ][ 1 ] * 
        getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                6 * ms.nCube, ! i, ms.fCrawford, 
                aafMET, aafMETPostCrawford );

      /* MWC for "No double; lose" */

      rNDL =
        (1.0 - aarRates[ ! i ][ 0 ] - aarRates[ ! i ][ 1 ]) *
        getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                1 * ms.nCube, ! i, ms.fCrawford,
                aafMET, aafMETPostCrawford )
        + aarRates[ ! i ][ 0 ] * 
        getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                2 * ms.nCube, ! i, ms.fCrawford, 
                aafMET, aafMETPostCrawford )
        + aarRates[ ! i ][ 1 ] * 
        getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                3 * ms.nCube, ! i, ms.fCrawford, 
                aafMET, aafMETPostCrawford );

      /* MWC for "Double, pass" */

      rDP = 
        getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                ms.nCube, i, ms.fCrawford,
                aafMET, aafMETPostCrawford );

      /* Double point */

      rRisk = rNDL - rDTL;
      rGain = rDTW - rNDW;

      arDP1 [ i ] = rRisk / ( rRisk + rGain );
      arDP2 [ i ] = arDP1 [ i ];

      /* Dead cube take point without redouble */

      rRisk = rDTW - rDP;
      rGain = rDP - rDTL;

      arCP1 [ i ] = 1.0 - rRisk / ( rRisk + rGain );

      if ( fAutoRedouble[ i ] ) {

        /* With redouble */

        rDTW =
          (1.0 - aarRates[ i ][ 0 ] - aarRates[ i ][ 1 ]) *
          getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                  4 * ms.nCube, i, ms.fCrawford,
                  aafMET, aafMETPostCrawford )
          + aarRates[ i ][ 0 ] * 
          getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                  8 * ms.nCube, i, ms.fCrawford,
                  aafMET, aafMETPostCrawford )
          + aarRates[ i ][ 1 ] * 
          getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                  12 * ms.nCube, i, ms.fCrawford,
                  aafMET, aafMETPostCrawford );

        rDTL =
          (1.0 - aarRates[ ! i ][ 0 ] - aarRates[ ! i ][ 1 ]) *
          getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                  4 * ms.nCube, ! i, ms.fCrawford,
                  aafMET, aafMETPostCrawford )
          + aarRates[ ! i ][ 0 ] * 
          getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                  8 * ms.nCube, ! i, ms.fCrawford,
                  aafMET, aafMETPostCrawford )
          + aarRates[ ! i ][ 1 ] * 
          getME ( ms.anScore[ 0 ], ms.anScore[ 1 ], ms.nMatchTo, i,
                  12 * ms.nCube, ! i, ms.fCrawford,
                  aafMET, aafMETPostCrawford );

        rRisk = rDTW - rDP;
        rGain = rDP - rDTL;

        arCP2 [ i ] = 1.0 - rRisk / ( rRisk + rGain );

        /* Double point */

        rRisk = rNDL - rDTL;
        rGain = rDTW - rNDW;
      
        arDP2 [ i ] = rRisk / ( rRisk + rGain );

      }
    }

    /* output */

    output ( "\n\n" );

    for ( i = 0; i < 2; i++ ) {

      outputf (_("Player %s market window:\n\n"), ap[ i ].szName );

      if ( fAutoRedouble[ i ] )
        output (_("Dead cube (opponent doesn't redouble): "));
      else
        output (_("Dead cube: "));

      outputf ("%6.2f%% - %6.2f%%\n", 100. * arDP1[ i ], 100. * arCP1[
                                                                      i ] );

      if ( fAutoRedouble[ i ] ) 
        outputf (_("Dead cube (opponent redoubles):"
                 "%6.2f%% - %6.2f%%\n\n"),
                 100. * arDP2[ i ], 100. * arCP2[ i ] );
      else if ( ! afDead[ i ] )
        outputf (_("Live cube:"
                 "%6.2f%% - %6.2f%%\n\n"),
                 100. * arDP2[ i ], 100. * arCP2[ i ] );

    }

  } /* ms.nMatchTo */
  else {

    /* money play: use Janowski's formulae */

    /* 
     * FIXME's: (1) make GTK version
     *          (2) make output for "current" value of X
     *          (3) improve layout?
     */

    const char *aszMoneyPointLabel[] = {
      N_("Take Point (TP)"),
      N_("Beaver Point (BP)"),
      N_("Raccoon Point (RP)"),
      N_("Initial Double Point (IDP)"),
      N_("Redouble Point (RDP)"),
      N_("Cash Point (CP)"),
      N_("Too good Point (TP)")
    };

    float aaarPoints[ 2 ][ 7 ][ 2 ];

    int i, j;

    getMoneyPoints ( aaarPoints, ci.fJacoby, ci.fBeavers, aarRates );

    for ( i = 0; i < 2; i++ ) {

      outputf (_("\nPlayer %s cube parameters:\n\n"), ap[ i ].szName );
      outputl ( _("Cube parameter               Dead Cube    Live Cube\n") );

      for ( j = 0; j < 7; j++ ) 
        outputf ( "%-27s  %7.3f%%     %7.3f%%\n",
                  gettext ( aszMoneyPointLabel[ j ] ),
                  aaarPoints[ i ][ j ][ 0 ] * 100.0f,
                  aaarPoints[ i ][ j ][ 1 ] * 100.0f );

    }

  }

}


extern void
CommandShowExport ( char *sz ) {

  int i;

#if USE_GTK 
  if( fX ) {
    GTKShowExport( &exsExport ); 
    return;
  }
#endif

  output ( _("\n" 
           "Export settings: \n\n"
           "WARNING: not all settings are honoured in the export!\n"
           "         Do not expect to much!\n\n"
           "Include: \n\n") );

  output ( _("- annotations") );
  outputf ( "\r\t\t\t\t: %s\n",
            exsExport.fIncludeAnnotation ? _("yes") : _("no") );
  output ( _("- analysis") );
  outputf ( "\r\t\t\t\t: %s\n",
            exsExport.fIncludeAnalysis ? _("yes") : _("no") );
  output ( _("- statistics") );
  outputf ( "\r\t\t\t\t: %s\n",
            exsExport.fIncludeStatistics ? _("yes") : _("no") );
  output ( _("- legend") );
  outputf ( "\r\t\t\t\t: %s\n",
            exsExport.fIncludeLegend ? _("yes") : _("no") );

  outputl ( _("Show: \n") );
  output ( _("- board" ) );
  output ( "\r\t\t\t\t: " );
  if ( ! exsExport.fDisplayBoard )
    outputl ( _("never") );
  else
    outputf ( _("on every %d move\n"), 
              exsExport.fDisplayBoard );

  output ( _("- players" ) );
  output ( "\r\t\t\t\t: " );
  if ( exsExport.fSide == 3 )
    outputl ( _("both") );
  else
    outputf ( "%s\n", 
              ap[ exsExport.fSide - 1 ].szName );

  outputl ( _("\nOutput moves:\n") );

  output ( _("- show at most" ) );
  output ( "\r\t\t\t\t: " );
  outputf ( _("%d moves\n"), exsExport.nMoves );

  output ( _("- show detailed probabilities" ) );
  output ( "\r\t\t\t\t: " );
  outputl ( exsExport.fMovesDetailProb ? _("yes") : _("no") );
  
  output ( _("- show evaluation parameters" ) );
  output ( "\r\t\t\t\t: " );
  outputl ( exsExport.afMovesParameters[ 0 ] ? _("yes") : _("no") );

  output ( _("- show rollout parameters" ) );
  output ( "\r\t\t\t\t: " );
  outputl ( exsExport.afMovesParameters[ 1 ] ? _("yes") : _("no") );

  for ( i = 0; i <= SKILL_VERYGOOD; i++ ) {
    if ( i == SKILL_NONE ) 
      output ( _("- unmarked moves" ) );
    else
      outputf ( _("- marked '%s'" ), gettext ( aszSkillType[ i ] ) );
    
    output ( "\r\t\t\t\t: " );
    outputl ( exsExport.afMovesDisplay[ i ] ? _("yes") : _("no") );
    
  }

  outputl ( _("\nOutput cube decisions:\n") );

  output ( _("- show detailed probabilities" ) );
  output ( "\r\t\t\t\t: " );
  outputl ( exsExport.fCubeDetailProb ? _("yes") : _("no") );
  
  output ( _("- show evaluation parameters" ) );
  output ( "\r\t\t\t\t: " );
  outputl ( exsExport.afCubeParameters[ 0 ] ? _("yes") : _("no") );

  output ( _("- show rollout parameters" ) );
  output ( "\r\t\t\t\t: " );
  outputl ( exsExport.afCubeParameters[ 1 ] ? _("yes") : _("no") );

  for ( i = 0; i <= SKILL_VERYGOOD; i++ ) {
    if ( i == SKILL_NONE ) 
      output ( _("- unmarked cube decisions" ) );
    else
      outputf ( _("- cube decisions marked '%s'" ), 
                gettext ( aszSkillType[ i ] ) );
    
    output ( "\r\t\t\t\t: " );
    outputl ( exsExport.afCubeDisplay[ i ] ? _("yes") : _("no") );

  }
  
  output ( _("- actual cube decisions" ) );
  output ( "\r\t\t\t\t: " );
  outputl ( exsExport.afCubeDisplay[ EXPORT_CUBE_ACTUAL ] ? 
            _("yes") : _("no") );

  output ( _("- missed cube decisions" ) );
  output ( "\r\t\t\t\t: " );
  outputl ( exsExport.afCubeDisplay[ EXPORT_CUBE_MISSED ] ? 
            _("yes") : _("no") );

  output ( _("- close cube decisions" ) );
  output ( "\r\t\t\t\t: " );
  outputl ( exsExport.afCubeDisplay[ EXPORT_CUBE_CLOSE ] ? 
            _("yes") : _("no") );

  outputl ( _("\nHTML options:\n") );

  outputf ( _("- URL to pictures used in export\n"
            "\t%s\n"),
            exsExport.szHTMLPictureURL ? exsExport.szHTMLPictureURL :
            _("not defined") );


  outputl ( "\n" );

}


extern void
CommandShowPath ( char *sz ) {

  int i;

  char *aszPathNames[] = {
    N_("Export of Encapsulated PostScript .eps files"),
    N_("Import or export of Jellyfish .gam files"),
    N_("Export of HTML files"),
    N_("Export of LaTeX files"),
    N_("Import or export of Jellyfish .mat files"),
    N_("Import of FIBS oldmoves files"),
    N_("Export of PDF files"),
    N_("Import of Jellyfish .pos files"),
    N_("Export of PostScript files"),
    N_("Load and save of SGF files"),
    N_("Import of GamesGrid SGG files"),
    N_("Loading of match equity files (.xml)")
  };

  /* make GTK widget that allows editing of paths */

#if USE_GTK
  if ( fX ) {
    GTKShowPath ();
    return;
  }
#endif

  outputl ( _("Default and current paths "
            "for load, save, import, and export: \n") );

  for ( i = 0; i <= PATH_SGG; ++i ) {

    outputf ( "%s:\n", gettext ( aszPathNames[ i ] ) );
    if ( ! strcmp ( aaszPaths[ i ][ 0 ], "" ) )
      outputl ( _("   Default : not defined") );
    else
      outputf ( _("   Default : \"%s\"\n"), aaszPaths[ i ][ 0 ] );

    if ( ! strcmp ( aaszPaths[ i ][ 1 ], "" ) )
      outputl ( _("   Current : not defined") );
    else
      outputf ( _("   Current : \"%s\"\n"), aaszPaths[ i ][ 1 ] );



  }

}
