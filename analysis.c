/*
 * analysis.c
 *
 * by Joern Thyssen <joern@thyssen.nu>, 2000
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
 * $Id: analysis.c,v 1.72 2002/07/19 07:37:42 thyssen Exp $
 */

#include "config.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "backgammon.h"
#include "drawboard.h"
#include "eval.h"
#if USE_GTK
#include "gtkgame.h"
#endif
#include "positionid.h"
#include "rollout.h"
#include "analysis.h"

#include "i18n.h"

const char *aszRating [ RAT_UNDEFINED + 1 ] = {
  N_("Beginner"), 
  N_("Novice"), 
  N_("Intermediate"), 
  N_("Advanced"), 
  N_("Expert"),
  N_("World class"), 
  N_("Extra-terrestrial"), 
  N_("N/A") };

const char *aszLuckRating[] = {
  N_("Haaa-haaa"),
  N_("Go to bed"), 
  N_("Better luck next time"),
  N_("None"),
  N_("Good dice, man!"),
  N_("Go to Las Vegas immediately"),
  N_("Cheater :-)")
};

static const float arThrsRating [ RAT_EXTRA_TERRESTRIAL + 1 ] = {
  1e38, 0.030, 0.025, 0.020, 0.015, 0.010, 0.005 };

extern ratingtype
GetRating ( const float rError ) {

  int i;

  for ( i = RAT_EXTRA_TERRESTRIAL; i >= 0; i-- )
    if ( rError < arThrsRating[ i ] ) return i;

  return RAT_UNDEFINED;
}

static float LuckAnalysis( int anBoard[ 2 ][ 25 ], int n0, int n1,
			   cubeinfo *pci, int fFirstMove ) {

    int anBoardTemp[ 2 ][ 25 ], i, j;
    float aar[ 6 ][ 6 ], ar[ NUM_OUTPUTS ], rMean = 0.0f;

    if( fFirstMove && n0 == n1 )
	fFirstMove = FALSE; /* games shouldn't start with a double */

    if( n0-- < n1-- )
	swap( &n0, &n1 );
    
    for( i = 0; i < 6; i++ )
	for( j = 0; fFirstMove ? j < i : j <= i; j++ ) {
	    memcpy( &anBoardTemp[ 0 ][ 0 ], &anBoard[ 0 ][ 0 ],
		    2 * 25 * sizeof( int ) );
	    
	    /* Find the best move for each roll at ply 0 only. */
	    if( FindBestMove( NULL, i + 1, j + 1, anBoardTemp, pci,
			      NULL ) < 0 )
		return ERR_VAL;
	    
	    SwapSides( anBoardTemp );

	    /* FIXME should we use EvaluatePositionCubeful here? */
	    if( EvaluatePosition( anBoardTemp, ar, pci, NULL ) )
		return ERR_VAL;

	    if( fFirstMove ) {
		rMean += Utility( ar, pci );
		InvertEvaluation( ar );
		rMean += aar[ i ][ j ] = Utility( ar, pci );
	    } else {
		InvertEvaluation( ar );
		aar[ i ][ j ] = Utility( ar, pci );
		
		rMean += ( i == j ) ? aar[ i ][ j ] : aar[ i ][ j ] * 2.0f;
	    }
	}

    return aar[ n0 ][ n1 ] - rMean / ( fFirstMove ? 30.0f : 36.0f );
}

static lucktype Luck( float r ) {

    if( r >= arLuckLevel[ LUCK_VERYGOOD ] )
	return LUCK_VERYGOOD;
    else if( r >= arLuckLevel[ LUCK_GOOD ] )
	return LUCK_GOOD;
    else if( r <= -arLuckLevel[ LUCK_VERYBAD ] )
	return LUCK_VERYBAD;
    else if( r <= -arLuckLevel[ LUCK_BAD ] )
	return LUCK_BAD;
    else
	return LUCK_NONE;
}

static skilltype Skill( float r ) {

    /* FIXME if the move is correct according to the selected evaluator
       but incorrect according to 0-ply, then return SKILL_GOOD or
       SKILL_VERYGOOD */

    if( r <= -arSkillLevel[ SKILL_VERYBAD ] )
	return SKILL_VERYBAD;
    else if( r <= -arSkillLevel[ SKILL_BAD ] )
	return SKILL_BAD;
    else if( r <= -arSkillLevel[ SKILL_DOUBTFUL ] )
	return SKILL_DOUBTFUL;
    else
	return SKILL_NONE;
}

/*
 * update statcontext for given move
 *
 * Input:
 *   psc: the statcontext to update
 *   pmr: the given move
 *   pms: the current match state
 *
 * Output:
 *   psc: the updated stat context
 *
 */

static void
updateStatcontext ( statcontext *psc,
                    moverecord *pmr,
                    matchstate *pms ) {

  cubeinfo ci;
  static unsigned char auch[ 10 ];
  float rSkill, rChequerSkill, rCost;
  int i;
  int anBoardMove[ 2 ][ 25 ];

  GetMatchStateCubeInfo ( &ci, pms );

  switch ( pmr->mt ) {

  case MOVE_GAMEINFO:
    /* no-op */
    break;

  case MOVE_NORMAL:

    /* 
     * Cube analysis; check for
     *   - missed doubles
     */

    if ( pmr->n.esDouble.et != EVAL_NONE && fAnalyseCube ) {

      float *arDouble = pmr->n.arDouble;

      psc->anTotalCube[ pmr->n.fPlayer ]++;
	  
      if( arDouble[ OUTPUT_NODOUBLE ] <
          arDouble[ OUTPUT_OPTIMAL ] ) {
        /* it was a double */
        rSkill = arDouble[ OUTPUT_NODOUBLE ] -
          arDouble[ OUTPUT_OPTIMAL ];
	      
        rCost = pms->nMatchTo ? eq2mwc( rSkill, &ci ) -
          eq2mwc( 0.0f, &ci ) : pms->nCube * rSkill;
	      
        if( arDouble[ OUTPUT_NODOUBLE ] >= 0.95 ) {
          /* around too good point */
          psc->anCubeMissedDoubleTG[ pmr->n.fPlayer ]++;
          psc->arErrorMissedDoubleTG[ pmr->n.fPlayer ][ 0 ] -=
            rSkill;
          psc->arErrorMissedDoubleTG[ pmr->n.fPlayer ][ 1 ] -=
            rCost;
        } 
        else {
          /* around double point */
          psc->anCubeMissedDoubleDP[ pmr->n.fPlayer ]++;
          psc->arErrorMissedDoubleDP[ pmr->n.fPlayer ][ 0 ] -=
            rSkill;
          psc->arErrorMissedDoubleDP[ pmr->n.fPlayer ][ 1 ] -=
            rCost;
        }
      
      } /* missed double */	

    } /* EVAL_NONE */


    /*
     * update luck statistics for roll
     */

    if ( fAnalyseDice && pmr->n.rLuck != ERR_VAL ) {

      psc->arLuck[ pmr->n.fPlayer ][ 0 ] += pmr->n.rLuck;
      psc->arLuck[ pmr->n.fPlayer ][ 1 ] += pms->nMatchTo ?
        eq2mwc( pmr->n.rLuck, &ci ) - eq2mwc( 0.0f, &ci ) :
        pms->nCube * pmr->n.rLuck;
      
      psc->anLuck[ pmr->n.fPlayer ][ pmr->n.lt ]++;

    }


    /*
     * update chequerplay statistics 
     */

    if ( fAnalyseMove && pmr->n.esChequer.et != EVAL_NONE ) {

      /* find skill */

      memcpy( anBoardMove, pms->anBoard, sizeof( anBoardMove ) );
      ApplyMove( anBoardMove, pmr->n.anMove, FALSE );
      PositionKey ( anBoardMove, auch );
      rChequerSkill = 0.0f;
	  
      for( i = 0; i < pmr->n.ml.cMoves; i++ ) 

        if( EqualKeys( auch,
                       pmr->n.ml.amMoves[ i ].auch ) ) {

          rChequerSkill =
            pmr->n.ml.amMoves[ i ].rScore - pmr->n.ml.amMoves[ 0 ].rScore;
          
          break;
        }

      /* update statistics */
	  
      rCost = pms->nMatchTo ? eq2mwc( rChequerSkill, &ci ) -
        eq2mwc( 0.0f, &ci ) : pms->nCube * rChequerSkill;
	  
      psc->anTotalMoves[ pmr->n.fPlayer ]++;
      psc->anMoves[ pmr->n.fPlayer ][ Skill( rChequerSkill ) ]++;
	  
      if( pmr->n.ml.cMoves > 1 ) {
        psc->anUnforcedMoves[ pmr->n.fPlayer ]++;
        psc->arErrorCheckerplay[ pmr->n.fPlayer ][ 0 ] -=
          rChequerSkill;
        psc->arErrorCheckerplay[ pmr->n.fPlayer ][ 1 ] -=
          rCost;
      }

    }

    break;

  case MOVE_DOUBLE:

    if ( fAnalyseCube && pmr->d.esDouble.et != EVAL_NONE ) {

      float *arDouble = pmr->d.arDouble;

      rSkill = arDouble[ OUTPUT_TAKE ] <
        arDouble[ OUTPUT_DROP ] ?
        arDouble[ OUTPUT_TAKE ] - arDouble[ OUTPUT_OPTIMAL ] :
        arDouble[ OUTPUT_DROP ] - arDouble[ OUTPUT_OPTIMAL ];
	      
      psc->anTotalCube[ pmr->d.fPlayer ]++;
      psc->anDouble[ pmr->d.fPlayer ]++;
	      
      if( rSkill < 0.0f ) {
        /* it was not a double */
        rCost = pms->nMatchTo ? eq2mwc( rSkill, &ci ) -
          eq2mwc( 0.0f, &ci ) : pms->nCube * rSkill;
		  
        if( arDouble[ OUTPUT_NODOUBLE ] >= 0.95f ) {
          /* around too good point */
          psc->anCubeWrongDoubleTG[ pmr->d.fPlayer ]++;
          psc->arErrorWrongDoubleTG[ pmr->d.fPlayer ][ 0 ] -=
            rSkill;
          psc->arErrorWrongDoubleTG[ pmr->d.fPlayer ][ 1 ] -=
            rCost;
        } else {
          /* around double point */
          psc->anCubeWrongDoubleDP[ pmr->d.fPlayer ]++;
          psc->arErrorWrongDoubleDP[ pmr->d.fPlayer ][ 0 ] -=
            rSkill;
          psc->arErrorWrongDoubleDP[ pmr->d.fPlayer ][ 1 ] -=
            rCost;
        }
      }

    }

    break;

  case MOVE_TAKE:

    if ( fAnalyseCube && pmr->d.esDouble.et != EVAL_NONE ) {

      float *arDouble = pmr->d.arDouble;

      psc->anTotalCube[ pmr->d.fPlayer ]++;
      psc->anTake[ pmr->d.fPlayer ]++;
	  
      if( -arDouble[ OUTPUT_TAKE ] < -arDouble[ OUTPUT_DROP ] ) {

        /* it was a drop */

        rSkill = -arDouble[ OUTPUT_TAKE ] - -arDouble[ OUTPUT_DROP ];
	      
        rCost = pms->nMatchTo ? eq2mwc( rSkill, &ci ) -
          eq2mwc( 0.0f, &ci ) : pms->nCube * rSkill;
	      
        psc->anCubeWrongTake[ pmr->d.fPlayer ]++;
        psc->arErrorWrongTake[ pmr->d.fPlayer ][ 0 ] -= rSkill;
        psc->arErrorWrongTake[ pmr->d.fPlayer ][ 1 ] -= rCost;

      }

    }

    break;

  case MOVE_DROP:

    if( fAnalyseCube && pmr->d.esDouble.et != EVAL_NONE ) {
	  
      float *arDouble = pmr->d.arDouble;

      psc->anTotalCube[ pmr->d.fPlayer ]++;
      psc->anPass[ pmr->d.fPlayer ]++;
	  
      if( -arDouble[ OUTPUT_DROP ] < -arDouble[ OUTPUT_TAKE ] ) {

        /* it was a take */
        
        rSkill = -arDouble[ OUTPUT_DROP ] - -arDouble[ OUTPUT_TAKE ];
	      
        rCost = pms->nMatchTo ? eq2mwc( rSkill, &ci ) -
          eq2mwc( 0.0f, &ci ) : pms->nCube * rSkill;
	      
        psc->anCubeWrongPass[ pmr->d.fPlayer ]++;
        psc->arErrorWrongPass[ pmr->d.fPlayer ][ 0 ] -= rSkill;
        psc->arErrorWrongPass[ pmr->d.fPlayer ][ 1 ] -= rCost;

      }

    }

    break;


  default:

    break;

  } /* switch */

}


extern int
AnalyzeMove ( moverecord *pmr, matchstate *pms, statcontext *psc,
              int fUpdateStatistics ) {

    static int anBoardMove[ 2 ][ 25 ];
    static int fFirstMove;
    static unsigned char auch[ 10 ];
    static cubeinfo ci;
    static float rSkill, rChequerSkill;
    static float aarOutput[ 2 ][ NUM_ROLLOUT_OUTPUTS ];
    static float aarStdDev[ 2 ][ NUM_ROLLOUT_OUTPUTS ];
    static rolloutstat aarsStatistics[ 2 ][ 2 ];
    static evalsetup esDouble; /* shared between the
				  double and subsequent take/drop */
    static float arDouble[ NUM_CUBEFUL_OUTPUTS ]; /* likewise */

    /* analyze this move */

    FixMatchState ( pms, pmr );

    switch( pmr->mt ) {
    case MOVE_GAMEINFO:
	fFirstMove = 1;

        if ( fUpdateStatistics )
          IniStatcontext( psc );
      
	break;
      
    case MOVE_NORMAL:
	if( pmr->n.fPlayer != pms->fMove ) {
	    SwapSides( pms->anBoard );
	    pms->fMove = pmr->n.fPlayer;
	}
      
	rSkill = rChequerSkill = 0.0f;
	GetMatchStateCubeInfo( &ci, pms );
      
	/* cube action? */
      
	if ( fAnalyseCube && !fFirstMove &&
	     GetDPEq ( NULL, NULL, &ci ) ) {
          
          if ( cmp_evalsetup ( &esAnalysisCube, &pmr->n.esDouble ) > 0 ) {
            
	    if ( GeneralCubeDecision ( "",
				       aarOutput, aarStdDev, aarsStatistics, 
				       pms->anBoard, &ci,
				       &esAnalysisCube ) < 0 )
              return -1;
            
            
	    FindCubeDecision ( arDouble, aarOutput, &ci );
            
	    pmr->n.esDouble = esAnalysisCube;

            memcpy ( pmr->n.arDouble, arDouble, sizeof ( arDouble ) );
            memcpy ( pmr->n.aarOutput, aarOutput, sizeof ( aarOutput ) );
            memcpy ( pmr->n.aarStdDev, aarStdDev, sizeof ( aarStdDev ) );
            
          }
          
          rSkill = pmr->n.arDouble[ OUTPUT_NODOUBLE ] -
            pmr->n.arDouble[ OUTPUT_OPTIMAL ];
	      
	    pmr->n.stCube = Skill( rSkill );

	} else
          pmr->n.esDouble.et = EVAL_NONE;

        /* luck analysis */
      
	if( fAnalyseDice ) {
	    pmr->n.rLuck = LuckAnalysis( pms->anBoard,
					 pmr->n.anRoll[ 0 ],
					 pmr->n.anRoll[ 1 ],
					 &ci, fFirstMove );
	    pmr->n.lt = Luck( pmr->n.rLuck );
	  
	}
      
	/* evaluate move */
      
	if( fAnalyseMove ) {
	    /* evaluate move */

	    memcpy( anBoardMove, pms->anBoard,
		    sizeof( anBoardMove ) );
	    ApplyMove( anBoardMove, pmr->n.anMove, FALSE );
	    PositionKey ( anBoardMove, auch );
	  
            if ( cmp_evalsetup ( &esAnalysisChequer, 
                                 &pmr->n.esChequer ) > 0 ) {

              if( pmr->n.ml.cMoves )
		free( pmr->n.ml.amMoves );
	  
              /* find best moves */
	  
              if( FindnSaveBestMoves ( &(pmr->n.ml), pmr->n.anRoll[ 0 ],
                                       pmr->n.anRoll[ 1 ],
                                       pms->anBoard, auch, &ci,
                                       &esAnalysisChequer.ec ) < 0 )
		return -1;

            }
	  
	    for( pmr->n.iMove = 0; pmr->n.iMove < pmr->n.ml.cMoves;
		 pmr->n.iMove++ )
		if( EqualKeys( auch,
			       pmr->n.ml.amMoves[ pmr->n.iMove ].auch ) ) {
		    rChequerSkill = pmr->n.ml.amMoves[ pmr->n.iMove ].
			rScore - pmr->n.ml.amMoves[ 0 ].rScore;
		  
		    break;
		}
	  
	    pmr->n.stMove = Skill( rChequerSkill );
	  
	    if( cAnalysisMoves >= 2 &&
		pmr->n.ml.cMoves > cAnalysisMoves ) {
		/* There are more legal moves than we want;
		   throw some away. */
		if( pmr->n.iMove >= cAnalysisMoves ) {
		    /* The move made wasn't in the top n; move it up so it
		       won't be discarded. */
		    memcpy( pmr->n.ml.amMoves + cAnalysisMoves - 1,
			    pmr->n.ml.amMoves + pmr->n.iMove,
			    sizeof( move ) );
		    pmr->n.iMove = cAnalysisMoves - 1;
		}
	      
                pmr->n.ml.amMoves = (move *)
                  realloc( pmr->n.ml.amMoves,
                           cAnalysisMoves * sizeof( move ) );
		pmr->n.ml.cMoves = cAnalysisMoves;
	    }

            pmr->n.esChequer = esAnalysisChequer;
            
	}
      
        if ( fUpdateStatistics )
          updateStatcontext ( psc, pmr, pms );
	  
	fFirstMove = 0;
      
	break;
      
    case MOVE_DOUBLE:
	if( pmr->d.fPlayer != pms->fMove ) {
	    SwapSides( pms->anBoard );
	    pms->fMove = pmr->d.fPlayer;
	}
      
	/* cube action */	    
	if( fAnalyseCube ) {
	    GetMatchStateCubeInfo( &ci, pms );
	  
	    if ( GetDPEq ( NULL, NULL, &ci ) ||
                 ci.fCubeOwner < 0 || ci.fCubeOwner == ci.fMove ) {
	      
              if ( cmp_evalsetup ( &esAnalysisCube, &pmr->d.esDouble ) > 0 ) {

		if ( GeneralCubeDecision ( "",
					   aarOutput, aarStdDev, aarsStatistics, 
					   pms->anBoard, &ci,
					   &esAnalysisCube ) < 0 )
		    return -1;

              }
              else {
                memcpy ( aarOutput, pmr->d.aarOutput, sizeof ( aarOutput ) );
                memcpy ( aarStdDev, pmr->d.aarStdDev, sizeof ( aarStdDev ) );
              }
	      
                FindCubeDecision ( arDouble, aarOutput, &ci );
	      
		esDouble = pmr->d.esDouble = esAnalysisCube;
	      
                memcpy ( pmr->d.arDouble, arDouble, sizeof ( arDouble ) );
                memcpy ( pmr->d.aarOutput, aarOutput, sizeof ( aarOutput ) );
                memcpy ( pmr->d.aarStdDev, aarStdDev, sizeof ( aarStdDev ) );
	      
		rSkill = arDouble[ OUTPUT_TAKE ] <
		    arDouble[ OUTPUT_DROP ] ?
		    arDouble[ OUTPUT_TAKE ] - arDouble[ OUTPUT_OPTIMAL ] :
		    arDouble[ OUTPUT_DROP ] - arDouble[ OUTPUT_OPTIMAL ];
	      
		pmr->d.st = Skill( rSkill );
	      
	    } else
		esDouble.et = EVAL_NONE;
	}
      
        if ( fUpdateStatistics )
          updateStatcontext ( psc, pmr, pms );

	break;
      
    case MOVE_TAKE:
      
	if( fAnalyseCube && esDouble.et != EVAL_NONE ) {

	    GetMatchStateCubeInfo( &ci, pms );
	  
	    pmr->d.esDouble = esDouble;

            memcpy ( pmr->d.arDouble, arDouble, sizeof ( arDouble ) );
            memcpy ( pmr->d.aarOutput, aarOutput, sizeof ( aarOutput ) );
            memcpy ( pmr->d.aarStdDev, aarStdDev, sizeof ( aarStdDev ) );
	  
            pmr->d.st = Skill ( -arDouble[ OUTPUT_TAKE ] - 
                                -arDouble[ OUTPUT_DROP ] );
	      
	}
        else
          pmr->d.esDouble.et = EVAL_NONE;

        if ( fUpdateStatistics )
          updateStatcontext ( psc, pmr, pms );

	break;
      
    case MOVE_DROP:
      
	if( fAnalyseCube && esDouble.et != EVAL_NONE ) {
	    GetMatchStateCubeInfo( &ci, pms );
	  
	    pmr->d.esDouble = esDouble;

	    memcpy( pmr->d.arDouble, arDouble, sizeof( arDouble ) );
            memcpy ( pmr->d.aarOutput, aarOutput, sizeof ( aarOutput ) );
            memcpy ( pmr->d.aarStdDev, aarStdDev, sizeof ( aarStdDev ) );
	  
            pmr->d.st = Skill( rSkill = -arDouble[ OUTPUT_DROP ] -
                               -arDouble[ OUTPUT_TAKE ] );
	      
	}
        else
          pmr->d.esDouble.et = EVAL_NONE;

        if ( fUpdateStatistics )
          updateStatcontext ( psc, pmr, pms );

	break;
      
    case MOVE_RESIGN:

      /* swap board if player not on roll resigned */

      if( pmr->r.fPlayer != pms->fMove ) {
        SwapSides( pms->anBoard );
        pms->fMove = pmr->n.fPlayer;
      }
      
      if ( esAnalysisCube.et != EVAL_NONE ) {
        
        int nResign;
        float rBefore, rAfter;

        GetMatchStateCubeInfo ( &ci, pms );

        if ( cmp_evalsetup ( &esAnalysisCube, &pmr->r.esResign ) > 0 ) {
          nResign =
            getResignation ( pmr->r.arResign, pms->anBoard, 
                             &ci, &esAnalysisCube );
          
        }

        getResignEquities ( pmr->r.arResign, &ci, pmr->r.nResigned,
                            &rBefore, &rAfter );

        pmr->r.esResign = esAnalysisCube;

        pmr->r.stResign = pmr->r.stAccept = SKILL_NONE;

        if ( rAfter < rBefore ) {
          /* wrong resign */
          pmr->r.stResign = Skill ( rAfter - rBefore );
          pmr->r.stAccept = SKILL_VERYGOOD;
        }

        if ( rBefore < rAfter ) {
          /* wrong accept */
          pmr->r.stAccept = Skill ( rBefore - rAfter );
          pmr->r.stResign = SKILL_VERYGOOD;
        }


      }

      break;
      
    case MOVE_SETDICE:
	if( pmr->sd.fPlayer != pms->fMove ) {
	    SwapSides( pms->anBoard );
	    pms->fMove = pmr->sd.fPlayer;
	}
      
	GetMatchStateCubeInfo( &ci, pms );
      
	if( fAnalyseDice ) {
	    pmr->sd.rLuck = LuckAnalysis( pms->anBoard,
					  pmr->sd.anDice[ 0 ],
					  pmr->sd.anDice[ 1 ],
					  &ci, fFirstMove );
	    pmr->sd.lt = Luck( pmr->sd.rLuck );
	}
      
	break;
      
    case MOVE_SETBOARD:	  
    case MOVE_SETCUBEVAL:
    case MOVE_SETCUBEPOS:
	break;
    }
  
    ApplyMoveRecord( pms, pmr );
  
    if ( fUpdateStatistics ) {
      psc->fMoves = fAnalyseMove;
      psc->fCube = fAnalyseCube;
      psc->fDice = fAnalyseDice;
    }
  
    return 0;
}


static int
AnalyzeGame ( list *plGame ) {

    list *pl;
    moverecord *pmr;
    movegameinfo *pmgi = plGame->plNext->p;
    matchstate msAnalyse;

    assert( pmgi->mt == MOVE_GAMEINFO );
    
    for( pl = plGame->plNext; pl != plGame; pl = pl->plNext ) {
	pmr = pl->p;

	ProgressValueAdd( 1 );

        if( AnalyzeMove ( pmr, &msAnalyse, &pmgi->sc, TRUE ) < 0 ) {
	    /* analysis incomplete; erase partial summary */
	    IniStatcontext( &pmgi->sc );
 	    return -1;
	}
    }
    
    return 0;
}

      	
extern void
AddStatcontext ( statcontext *pscA, statcontext *pscB ) {

  /* pscB = pscB + pscA */

  int i, j;

  pscB->fMoves |= pscA->fMoves;
  pscB->fDice |= pscA->fDice;
  pscB->fCube |= pscA->fCube;
  
  for ( i = 0; i < 2; i++ ) {

    pscB->anUnforcedMoves[ i ] += pscA->anUnforcedMoves[ i ];
    pscB->anTotalMoves[ i ] += pscA->anTotalMoves[ i ];
    
    pscB->anTotalCube[ i ] += pscA->anTotalCube[ i ];
    pscB->anDouble[ i ] += pscA->anDouble[ i ];
    pscB->anTake[ i ] += pscA->anTake[ i ];
    pscB->anPass[ i ] += pscA->anPass[ i ];

    for ( j = 0; j <= SKILL_VERYGOOD; j++ )
      pscB->anMoves[ i ][ j ] += pscA->anMoves[ i ][ j ];

    for ( j = 0; j <= LUCK_VERYGOOD; j++ )
      pscB->anLuck[ i ][ j ] += pscA->anLuck[ i ][ j ];

    pscB->anCubeMissedDoubleDP [ i ] += pscA->anCubeMissedDoubleDP [ i ];
    pscB->anCubeMissedDoubleTG [ i ] += pscA->anCubeMissedDoubleTG [ i ];
    pscB->anCubeWrongDoubleDP [ i ] += pscA->anCubeWrongDoubleDP [ i ];
    pscB->anCubeWrongDoubleTG [ i ] += pscA->anCubeWrongDoubleTG [ i ];
    pscB->anCubeWrongTake [ i ] += pscA->anCubeWrongTake [ i ];
    pscB->anCubeWrongPass [ i ] += pscA->anCubeWrongPass [ i ];

    for ( j = 0; j < 2; j++ ) {

      pscB->arErrorCheckerplay [ i ][ j ] +=
        pscA->arErrorCheckerplay[ i ][ j ];
      pscB->arErrorMissedDoubleDP [ i ][ j ] +=
        pscA->arErrorMissedDoubleDP[ i ][ j ];
      pscB->arErrorMissedDoubleTG [ i ][ j ] +=
        pscA->arErrorMissedDoubleTG[ i ][ j ];
      pscB->arErrorWrongDoubleDP [ i ][ j ] +=
        pscA->arErrorWrongDoubleDP[ i ][ j ];
      pscB->arErrorWrongDoubleTG [ i ][ j ] +=
        pscA->arErrorWrongDoubleTG[ i ][ j ];
      pscB->arErrorWrongTake [ i ][ j ] +=
        pscA->arErrorWrongTake[ i ][ j ];
      pscB->arErrorWrongPass [ i ][ j ] +=
        pscA->arErrorWrongPass [ i ][ j ];
      pscB->arLuck [ i ][ j ] +=
        pscA->arLuck [ i ][ j ];

    }

  }

}

static int CheckSettings( void ) {

    if( !fAnalyseCube && !fAnalyseDice && !fAnalyseMove ) {
	outputl( _("You must specify at least one type of analysis to perform "
		 "(see `help set\nanalysis').") );
	return -1;
    }

    return 0;
}

static int
NumberMovesGame ( list *plGame ) {

  int nMoves = 0;
  list *pl;

  for( pl = plGame->plNext; pl != plGame; pl = pl->plNext ) 
    nMoves++;

  return nMoves;

}


static int
NumberMovesMatch ( list *plMatch ) {

  int nMoves = 0;
  list *pl;

  for ( pl = plMatch->plNext; pl != plMatch; pl = pl->plNext )
    nMoves += NumberMovesGame ( pl->p );

  return nMoves;

}


extern void CommandAnalyseGame( char *sz ) {

  int nMoves;
  
  if( !plGame ) {
    outputl( _("No game is being played.") );
    return;
  }
    
  if( CheckSettings() )
    return;
  
  nMoves = NumberMovesGame ( plGame );

  ProgressStartValue( _("Analysing game; move:"), nMoves );
    
  AnalyzeGame( plGame );
  
  ProgressEnd();

#if USE_GTK
  if( fX )
    GTKUpdateAnnotations();
#endif
}


extern void CommandAnalyseMatch( char *sz ) {

  list *pl;
  movegameinfo *pmgi;
  int nMoves;
  
  if( ListEmpty( &lMatch ) ) {
      outputl( _("No match is being played.") );
      return;
  }

  if( CheckSettings() )
      return;

  nMoves = NumberMovesMatch ( &lMatch );

  ProgressStartValue( _("Analysing match; move:"), nMoves );

  IniStatcontext( &scMatch );
  
  for( pl = lMatch.plNext; pl != &lMatch; pl = pl->plNext ) {
      if( AnalyzeGame( pl->p ) < 0 ) {
	  /* analysis incomplete; erase partial summary */
	  IniStatcontext( &scMatch );
	  break;
      }
      pmgi = ( (list *) pl->p )->plNext->p;
      assert( pmgi->mt == MOVE_GAMEINFO );
      AddStatcontext( &pmgi->sc, &scMatch );
  }
  
  ProgressEnd();

#if USE_GTK
  if( fX )
      GTKUpdateAnnotations();
#endif
}

extern void CommandAnalyseSession( char *sz ) {

    CommandAnalyseMatch( sz );
}



extern void
IniStatcontext ( statcontext *psc ) {

  /* Initialize statcontext with all zeroes */

  int i, j;

  psc->fMoves = psc->fCube = psc->fDice = FALSE;
  
  for ( i = 0; i < 2; i++ ) {

    psc->anUnforcedMoves[ i ] = 0;
    psc->anTotalMoves[ i ] = 0;
    
    psc->anTotalCube[ i ] = 0;
    psc->anDouble[ i ] = 0;
    psc->anTake[ i ] = 0;
    psc->anPass[ i ] = 0;

    for ( j = 0; j <= SKILL_VERYGOOD; j++ )
      psc->anMoves[ i ][ j ] = 0;

    for ( j = 0; j <= LUCK_VERYGOOD; j++ )
      psc->anLuck[ i ][ j ] = 0;

    psc->anCubeMissedDoubleDP [ i ] = 0;
    psc->anCubeMissedDoubleTG [ i ] = 0;
    psc->anCubeWrongDoubleDP [ i ] = 0;
    psc->anCubeWrongDoubleTG [ i ] = 0;
    psc->anCubeWrongTake [ i ] = 0;
    psc->anCubeWrongPass [ i ] = 0;

    for ( j = 0; j < 2; j++ ) {

      psc->arErrorCheckerplay [ i ][ j ] = 0.0;
      psc->arErrorMissedDoubleDP [ i ][ j ] = 0.0;
      psc->arErrorMissedDoubleTG [ i ][ j ] = 0.0;
      psc->arErrorWrongDoubleDP [ i ][ j ] = 0.0;
      psc->arErrorWrongDoubleTG [ i ][ j ] = 0.0;
      psc->arErrorWrongTake[ i ][ j ] = 0.0;
      psc->arErrorWrongPass[ i ][ j ] = 0.0;
      psc->arLuck[ i ][ j ] = 0.0;

    }

  }

}



extern float
relativeFibsRating ( const float r, const int n ) {

  return - 2000.0 / sqrt ( 1.0 * n ) * log10 ( 1.0 / r - 1.0 );

} 


/*
 * Calculate an estimated rating based on some empirical estimate
 * using linear interpolation.
 *
 * Input
 *    r: error rate pr. move
 *    n: match length
 *
 */

extern float
absoluteFibsRating ( const float r, const int n ) {

  static float arRating[] =
    {  500, 1500,  1600,  1750,  1850,  1950,  2050, 2200 };
  static float arErrorRate[] =
    { 1e38, 0.030, 0.025, 0.020, 0.015, 0.010, 0.005, 0.0 };
  int i;
  float a;

  if ( r < 0 )
    return 2200;

  for ( i = 6; i >= 0; i-- ) {

    if ( r < arErrorRate[ i ] ) {

      a = ( arRating[ i + 1 ] -  arRating[ i ] ) / 
        ( arErrorRate[ i + 1 ] - arErrorRate[ i ] );

      return arRating[ i ] + a * ( r - arErrorRate[ i ] );

    }

  }

  return 500;

}


extern float
getMWCFromError ( const statcontext *psc, float aaaar[ 3 ][ 2 ][ 2 ][ 2 ] ) {

  int i, j;

  for ( i = 0; i < 2; i++ ) 
    for ( j = 0; j < 2; j++ ) {

      /* chequer play */

      aaaar[ CHEQUERPLAY ][ TOTAL ][ i ][ j ] = psc->arErrorCheckerplay[ i ][ j ];
      aaaar[ CHEQUERPLAY ][ PERMOVE ][ i ][ j ] = 
        aaaar[ 0 ][ 0 ][ i ][ j ] / psc->anUnforcedMoves[ i ];

      /* cube decisions */

      aaaar[ CUBEDECISION ][ TOTAL ][ i ][ j ] = 
        psc->arErrorMissedDoubleDP[ i ][ j ]
        + psc->arErrorMissedDoubleTG[ i ][ j ]
        + psc->arErrorWrongDoubleDP[ i ][ j ]
        + psc->arErrorWrongDoubleTG[ i ][ j ]
        + psc->arErrorWrongTake[ i ][ j ]
        + psc->arErrorWrongPass[ i ][ j ];
      
      aaaar[ CUBEDECISION ][ PERMOVE ][ i ][ j ] =
        aaaar[ CUBEDECISION ][ TOTAL ][ i ][ j ] / psc->anTotalCube[ i ];

      /* sum chequer play and cube decisions */
      /* FIXME: what average should be used? */

      aaaar[ COMBINED ][ TOTAL ][ i ][ j ] =
        aaaar[ CHEQUERPLAY ][ TOTAL ][ i ][ j ] + aaaar[ CUBEDECISION ][ TOTAL ][ i ][ j ];

      aaaar[ COMBINED ][ PERMOVE ][ i ][ j ] =
        aaaar[ COMBINED ][ TOTAL ][ i ][ j ] / 
        ( psc->anUnforcedMoves[ i ] + psc->anTotalCube[ i ] );

    }

  return 0.50 
    - aaaar[ COMBINED ][ TOTAL ][ PLAYER_0 ][ UNNORMALISED ] 
    + aaaar[ COMBINED ][ TOTAL ][ PLAYER_1 ][ UNNORMALISED ];

}


extern void
DumpStatcontext ( char *szOutput, const statcontext *psc, const char * sz ) {

  int i;
  ratingtype rt[ 2 ];
  char szTemp[1024];
  float aaaar[ 3 ][ 2 ][ 2 ][ 2 ];
  float r = getMWCFromError ( psc, aaaar );


  /* nice human readable dump */

  /* FIXME: make tty output shorter */
  /* FIXME: the code below is only for match play */
  /* FIXME: honour fOutputMWC etc. */
  /* FIXME: calculate ratings (ET, World class, etc.) */
  /* FIXME: use output*() functions, not printf */

  sprintf ( szTemp, "Player\t\t\t\t%-15s\t\t%-15s\n\n",
           ap[ 0 ].szName, ap [ 1 ].szName );
  strcpy ( szOutput, szTemp);
 
  if( psc->fMoves ) {
      sprintf( szTemp, _("Checkerplay statistics:\n\n"
	      "Total moves:\t\t\t%3d\t\t\t%3d\n"
	      "Unforced moves:\t\t\t%3d\t\t\t%3d\n\n"
	      "Moves marked very good\t\t%3d\t\t\t%3d\n"
	      "Moves marked good\t\t%3d\t\t\t%3d\n"
	      "Moves marked interesting\t%3d\t\t\t%3d\n"
	      "Moves unmarked\t\t\t%3d\t\t\t%3d\n"
	      "Moves marked doubtful\t\t%3d\t\t\t%3d\n"
	      "Moves marked bad\t\t%3d\t\t\t%3d\n"
	      "Moves marked very bad\t\t%3d\t\t\t%3d\n\n"),
	      psc->anTotalMoves[ 0 ], psc->anTotalMoves[ 1 ],
	      psc->anUnforcedMoves[ 0 ], psc->anUnforcedMoves[ 1 ],
	      psc->anMoves[ 0 ][ SKILL_VERYGOOD ],
	      psc->anMoves[ 1 ][ SKILL_VERYGOOD ],
	      psc->anMoves[ 0 ][ SKILL_GOOD ],
	      psc->anMoves[ 1 ][ SKILL_GOOD ],
	      psc->anMoves[ 0 ][ SKILL_INTERESTING ],
	      psc->anMoves[ 1 ][ SKILL_INTERESTING ],
	      psc->anMoves[ 0 ][ SKILL_NONE ],
	      psc->anMoves[ 1 ][ SKILL_NONE ],
	      psc->anMoves[ 0 ][ SKILL_DOUBTFUL ],
	      psc->anMoves[ 1 ][ SKILL_DOUBTFUL ],
	      psc->anMoves[ 0 ][ SKILL_BAD ],
	      psc->anMoves[ 1 ][ SKILL_BAD ],
	      psc->anMoves[ 0 ][ SKILL_VERYBAD ],
	      psc->anMoves[ 1 ][ SKILL_VERYBAD ] );
      strcat ( szOutput, szTemp);

      if ( ms.nMatchTo ){
	  sprintf ( szTemp,_("Error rate (total)\t\t"
                             "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n"
                             "Error rate (pr. move)\t\t"
                             "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n\n"),
                    -aaaar[ CHEQUERPLAY ][ TOTAL ][ PLAYER_0 ][ NORMALISED ],
                    -aaaar[ CHEQUERPLAY ][ TOTAL ][ PLAYER_0 ][ UNNORMALISED ] * 100.0f,
                    
                    -aaaar[ CHEQUERPLAY ][ TOTAL ][ PLAYER_1 ][ NORMALISED ],
                    -aaaar[ CHEQUERPLAY ][ TOTAL ][ PLAYER_1 ][ UNNORMALISED ] * 100.0f,
                    
                    -aaaar[ CHEQUERPLAY ][ PERMOVE ][ PLAYER_0 ][ NORMALISED ],
                    -aaaar[ CHEQUERPLAY ][ PERMOVE ][ PLAYER_0 ][ UNNORMALISED ] * 100.0f,
                    
                    -aaaar[ CHEQUERPLAY ][ PERMOVE ][ PLAYER_1 ][ NORMALISED ],
                    -aaaar[ CHEQUERPLAY ][ PERMOVE ][ PLAYER_1 ][ UNNORMALISED ] * 100.0f
                    );
          strcat ( szOutput, szTemp);
      } else {
	  sprintf ( szTemp,_("Error rate (total)\t\t"
                             "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n"
                             "Error rate (pr. move)\t\t"
                             "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n\n"),
                    -aaaar[ CHEQUERPLAY ][ TOTAL ][ PLAYER_0 ][ NORMALISED ],
                    -aaaar[ CHEQUERPLAY ][ TOTAL ][ PLAYER_0 ][ UNNORMALISED ],
                    
                    -aaaar[ CHEQUERPLAY ][ TOTAL ][ PLAYER_1 ][ NORMALISED ],
                    -aaaar[ CHEQUERPLAY ][ TOTAL ][ PLAYER_1 ][ UNNORMALISED ],
                    
                    -aaaar[ CHEQUERPLAY ][ PERMOVE ][ PLAYER_0 ][ NORMALISED ],
                    -aaaar[ CHEQUERPLAY ][ PERMOVE ][ PLAYER_0 ][ UNNORMALISED ],
                    
                    -aaaar[ CHEQUERPLAY ][ PERMOVE ][ PLAYER_1 ][ NORMALISED ],
                    -aaaar[ CHEQUERPLAY ][ PERMOVE ][ PLAYER_1 ][ UNNORMALISED ] 
                    );
          strcat ( szOutput, szTemp);
      }

      for ( i = 0 ; i < 2; i++ )
        rt[ i ] = GetRating ( aaaar[ CHEQUERPLAY ][ PERMOVE ][ i ][ NORMALISED ] );
      
      sprintf ( szTemp, _("Checker play rating:\t\t%-20s\t%-20s\n\n"),
                gettext ( aszRating[ rt [ 0 ] ] ), 
                gettext ( aszRating[ rt [ 1 ] ] ) );
      strcat ( szOutput, szTemp);
  }

  if( psc->fDice ) {
      sprintf ( szTemp, _("Rolls marked very lucky\t\t%3d\t\t\t%3d\n"
	       "Rolls marked lucky\t\t%3d\t\t\t%3d\n"
	       "Rolls unmarked\t\t\t%3d\t\t\t%3d\n"
	       "Rolls marked unlucky\t\t%3d\t\t\t%3d\n"
	       "Rolls marked very unlucky\t%3d\t\t\t%3d\n"),
	       psc->anLuck[ 0 ][ LUCK_VERYGOOD ],
	       psc->anLuck[ 1 ][ LUCK_VERYGOOD ],
	       psc->anLuck[ 0 ][ LUCK_GOOD ],
	       psc->anLuck[ 1 ][ LUCK_GOOD ],
	       psc->anLuck[ 0 ][ LUCK_NONE ],
	       psc->anLuck[ 1 ][ LUCK_NONE ],
	       psc->anLuck[ 0 ][ LUCK_BAD ],
	       psc->anLuck[ 1 ][ LUCK_BAD ],
	       psc->anLuck[ 0 ][ LUCK_VERYBAD ],
	       psc->anLuck[ 1 ][ LUCK_VERYBAD ] );
      strcat ( szOutput, szTemp);
       
      if ( ms.nMatchTo ){
	  sprintf ( szTemp,_("Luck rate (total)\t\t"
		  "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n"
		  "Luck rate (pr. move)\t\t"
		  "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n\n"),
		  psc->arLuck[ 0 ][ 0 ],
		  psc->arLuck[ 0 ][ 1 ] * 100.0f,
		  psc->arLuck[ 1 ][ 0 ],
		  psc->arLuck[ 1 ][ 1 ] * 100.0f,
		  psc->arLuck[ 0 ][ 0 ] /
		  psc->anTotalMoves[ 0 ],
		  psc->arLuck[ 0 ][ 1 ] * 100.0f /
		  psc->anTotalMoves[ 0 ],
		  psc->arLuck[ 1 ][ 0 ] /
		  psc->anTotalMoves[ 1 ],
		  psc->arLuck[ 1 ][ 1 ] * 100.0f /
		  psc->anTotalMoves[ 1 ] );
          strcat ( szOutput, szTemp);
      } else {
	  sprintf ( szTemp,_("Luck rate (total)\t\t"
		  "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n"
		  "Luck rate (pr. move)\t\t"
		  "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n\n"),
		  psc->arLuck[ 0 ][ 0 ],
		  psc->arLuck[ 0 ][ 1 ],
		  psc->arLuck[ 1 ][ 0 ],
		  psc->arLuck[ 1 ][ 1 ],
		  psc->arLuck[ 0 ][ 0 ] /
		  psc->anTotalMoves[ 0 ],
		  psc->arLuck[ 0 ][ 1 ] /
		  psc->anTotalMoves[ 0 ],
		  psc->arLuck[ 1 ][ 0 ] /
		  psc->anTotalMoves[ 1 ],
		  psc->arLuck[ 1 ][ 1 ] /
		  psc->anTotalMoves[ 1 ] );
          strcat ( szOutput, szTemp);
      }

      /* luck rating */


      for ( i = 0; i < 2; i++ ) 
        rt[ i ] = getLuckRating ( psc->arLuck[ i ][ 0 ] /
                                  psc->anTotalMoves[ i ] );

      sprintf ( szTemp, _("Luck rating:\t\t\t%-15s\t\t%-15s\n\n"),
                gettext ( aszLuckRating[ rt [ 0 ] ] ), 
                gettext ( aszLuckRating[ rt [ 1 ] ] ) );
      strcat ( szOutput, szTemp);
      

  }

  if( psc->fCube ) {
      sprintf ( szTemp, _("\nCube decisions statistics:\n\n") );
      strcat ( szOutput, szTemp);

      sprintf ( szTemp, _("Total cube decisions\t\t%3d\t\t\t%3d\n"
	       "Doubles\t\t\t\t%3d\t\t\t%3d\n"
	       "Takes\t\t\t\t%3d\t\t\t%3d\n"
	       "Pass\t\t\t\t%3d\t\t\t%3d\n\n"),
	       psc->anTotalCube[ 0 ],
	       psc->anTotalCube[ 1 ],
	       psc->anDouble[ 0 ], 
	       psc->anDouble[ 1 ], 
	       psc->anTake[ 0 ],
	       psc->anTake[ 1 ],
	       psc->anPass[ 0 ], 
	       psc->anPass[ 1 ] );
      strcat ( szOutput, szTemp);
      
      if ( ms.nMatchTo ){
        sprintf ( szTemp,_("Missed doubles around DP\t"
		  "%3d (%+6.3f (%+7.3f%%)\t%3d (%+6.3f (%+7.3f%%)\n"
		  "Missed doubles around TG\t"
		  "%3d (%+6.3f (%+7.3f%%)\t%3d (%+6.3f (%+7.3f%%)\n"
		  "Wrong doubles around DP\t\t"
		  "%3d (%+6.3f (%+7.3f%%)\t%3d (%+6.3f (%+7.3f%%)\n"
		  "Wrong doubles around TG\t\t"
		  "%3d (%+6.3f (%+7.3f%%)\t%3d (%+6.3f (%+7.3f%%)\n"
		  "Wrong takes\t\t\t"
		  "%3d (%+6.3f (%+7.3f%%)\t%3d (%+6.3f (%+7.3f%%)\n"
		  "Wrong passes\t\t\t"
		  "%3d (%+6.3f (%+7.3f%%)\t%3d (%+6.3f (%+7.3f%%)\n"),
                  psc->anCubeMissedDoubleDP[ 0 ],
		  -psc->arErrorMissedDoubleDP[ 0 ][ 0 ],
		  -psc->arErrorMissedDoubleDP[ 0 ][ 1 ] * 100.0f,
		  psc->anCubeMissedDoubleDP[ 1 ],
		  -psc->arErrorMissedDoubleDP[ 1 ][ 0 ],
		  -psc->arErrorMissedDoubleDP[ 1 ][ 1 ] * 100.0f,
		  psc->anCubeMissedDoubleTG[ 0 ],
		  -psc->arErrorMissedDoubleTG[ 0 ][ 0 ],
		  -psc->arErrorMissedDoubleTG[ 0 ][ 1 ] * 100.0f,
		  psc->anCubeMissedDoubleTG[ 1 ],
		  -psc->arErrorMissedDoubleTG[ 1 ][ 0 ],
		  -psc->arErrorMissedDoubleTG[ 1 ][ 1 ] * 100.0f,
		  psc->anCubeWrongDoubleDP[ 0 ],
		  -psc->arErrorWrongDoubleDP[ 0 ][ 0 ],
		  -psc->arErrorWrongDoubleDP[ 0 ][ 1 ] * 100.0f,
		  psc->anCubeWrongDoubleDP[ 1 ],
		  -psc->arErrorWrongDoubleDP[ 1 ][ 0 ],
		  -psc->arErrorWrongDoubleDP[ 1 ][ 1 ] * 100.0f,
		  psc->anCubeWrongDoubleTG[ 0 ],
		  -psc->arErrorWrongDoubleTG[ 0 ][ 0 ],
		  -psc->arErrorWrongDoubleTG[ 0 ][ 1 ] * 100.0f,
		  psc->anCubeWrongDoubleTG[ 1 ],
		  -psc->arErrorWrongDoubleTG[ 1 ][ 0 ],
		  -psc->arErrorWrongDoubleTG[ 1 ][ 1 ] * 100.0f,
		  psc->anCubeWrongTake[ 0 ],
		  -psc->arErrorWrongTake[ 0 ][ 0 ],
		  -psc->arErrorWrongTake[ 0 ][ 1 ] * 100.0f,
		  psc->anCubeWrongTake[ 1 ],
		  -psc->arErrorWrongTake[ 1 ][ 0 ],
		  -psc->arErrorWrongTake[ 1 ][ 1 ] * 100.0f,
		  psc->anCubeWrongPass[ 0 ],
		  -psc->arErrorWrongPass[ 0 ][ 0 ],
		  -psc->arErrorWrongPass[ 0 ][ 1 ] * 100.0f,
		  psc->anCubeWrongPass[ 1 ],
		  -psc->arErrorWrongPass[ 1 ][ 0 ],
		  -psc->arErrorWrongPass[ 1 ][ 1 ] * 100.0f );
          strcat ( szOutput, szTemp);
      } else {
	  sprintf ( szTemp,_("Missed doubles around DP\t"
		  "%3d (%+6.3f (%+7.3f%%)\t%3d (%+6.3f (%+7.3f%%)\n"
		  "Missed doubles around TG\t"
		  "%3d (%+6.3f (%+7.3f%%)\t%3d (%+6.3f (%+7.3f%%)\n"
		  "Wrong doubles around DP\t\t"
		  "%3d (%+6.3f (%+7.3f%%)\t%3d (%+6.3f (%+7.3f%%)\n"
		  "Wrong doubles around TG\t\t"
		  "%3d (%+6.3f (%+7.3f%%)\t%3d (%+6.3f (%+7.3f%%)\n"
		  "Wrong takes\t\t\t"
		  "%3d (%+6.3f (%+7.3f%%)\t%3d (%+6.3f (%+7.3f%%)\n"
		  "Wrong passes\t\t\t"
		  "%3d (%+6.3f (%+7.3f%%)\t%3d (%+6.3f (%+7.3f%%)\n"),
		  psc->anCubeMissedDoubleDP[ 0 ],
		  -psc->arErrorMissedDoubleDP[ 0 ][ 0 ],
		  -psc->arErrorMissedDoubleDP[ 0 ][ 1 ],
		  psc->anCubeMissedDoubleDP[ 1 ],
		  -psc->arErrorMissedDoubleDP[ 1 ][ 0 ],
		  -psc->arErrorMissedDoubleDP[ 1 ][ 1 ],
		  psc->anCubeMissedDoubleTG[ 0 ],
		  -psc->arErrorMissedDoubleTG[ 0 ][ 0 ],
		  -psc->arErrorMissedDoubleTG[ 0 ][ 1 ],
		  psc->anCubeMissedDoubleTG[ 1 ],
		  -psc->arErrorMissedDoubleTG[ 1 ][ 0 ],
		  -psc->arErrorMissedDoubleTG[ 1 ][ 1 ],
		  psc->anCubeWrongDoubleDP[ 0 ],
		  -psc->arErrorWrongDoubleDP[ 0 ][ 0 ],
		  -psc->arErrorWrongDoubleDP[ 0 ][ 1 ],
		  psc->anCubeWrongDoubleDP[ 1 ],
		  -psc->arErrorWrongDoubleDP[ 1 ][ 0 ],
		  -psc->arErrorWrongDoubleDP[ 1 ][ 1 ],
		  psc->anCubeWrongDoubleTG[ 0 ],
		  -psc->arErrorWrongDoubleTG[ 0 ][ 0 ],
		  -psc->arErrorWrongDoubleTG[ 0 ][ 1 ],
		  psc->anCubeWrongDoubleTG[ 1 ],
		  -psc->arErrorWrongDoubleTG[ 1 ][ 0 ],
		  -psc->arErrorWrongDoubleTG[ 1 ][ 1 ],
		  psc->anCubeWrongTake[ 0 ],
		  -psc->arErrorWrongTake[ 0 ][ 0 ],
		  -psc->arErrorWrongTake[ 0 ][ 1 ],
		  psc->anCubeWrongTake[ 1 ],
		  -psc->arErrorWrongTake[ 1 ][ 0 ],
		  -psc->arErrorWrongTake[ 1 ][ 1 ],
		  psc->anCubeWrongPass[ 0 ],
		  -psc->arErrorWrongPass[ 0 ][ 0 ],
		  -psc->arErrorWrongPass[ 0 ][ 1 ],
		  psc->anCubeWrongPass[ 1 ],
		  -psc->arErrorWrongPass[ 1 ][ 0 ],
		  -psc->arErrorWrongPass[ 1 ][ 1 ] );
          strcat ( szOutput, szTemp);
      }

      if ( ms.nMatchTo ){
	  sprintf ( szTemp,_("Error rate (total)\t\t"
                             "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n"
                             "Error rate (per cube decision)\t"
                             "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n"),
                    -aaaar[ CUBEDECISION ][ TOTAL ][ PLAYER_0 ][ NORMALISED ],
                    -aaaar[ CUBEDECISION ][ TOTAL ][ PLAYER_0 ][ UNNORMALISED ] * 100.0f,
                    
                    -aaaar[ CUBEDECISION ][ TOTAL ][ PLAYER_1 ][ NORMALISED ],
                    -aaaar[ CUBEDECISION ][ TOTAL ][ PLAYER_1 ][ UNNORMALISED ] * 100.0f,
                    
                    -aaaar[ CUBEDECISION ][ PERMOVE ][ PLAYER_0 ][ NORMALISED ],
                    -aaaar[ CUBEDECISION ][ PERMOVE ][ PLAYER_0 ][ UNNORMALISED ] * 100.0f,
                    
                    -aaaar[ CUBEDECISION ][ PERMOVE ][ PLAYER_1 ][ NORMALISED ],
                    -aaaar[ CUBEDECISION ][ PERMOVE ][ PLAYER_1 ][ UNNORMALISED ] * 100.0f
                    );
          strcat ( szOutput, szTemp);
      } else {
	  sprintf ( szTemp,_("Error rate (total)\t\t"
                             "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n"
                             "Error rate (per cube decision)\t"
                             "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n"),
                    -aaaar[ CUBEDECISION ][ TOTAL ][ PLAYER_0 ][ NORMALISED ],
                    -aaaar[ CUBEDECISION ][ TOTAL ][ PLAYER_0 ][ UNNORMALISED ],
                    
                    -aaaar[ CUBEDECISION ][ TOTAL ][ PLAYER_1 ][ NORMALISED ],
                    -aaaar[ CUBEDECISION ][ TOTAL ][ PLAYER_1 ][ UNNORMALISED ],
                    
                    -aaaar[ CUBEDECISION ][ PERMOVE ][ PLAYER_0 ][ NORMALISED ],
                    -aaaar[ CUBEDECISION ][ PERMOVE ][ PLAYER_0 ][ UNNORMALISED ],
                    
                    -aaaar[ CUBEDECISION ][ PERMOVE ][ PLAYER_1 ][ NORMALISED ],
                    -aaaar[ CUBEDECISION ][ PERMOVE ][ PLAYER_1 ][ UNNORMALISED ] 
                    );
          strcat ( szOutput, szTemp);
      }


      for ( i = 0 ; i < 2; i++ )
        rt[ i ] = GetRating ( aaaar[ CUBEDECISION ][ PERMOVE ][ i ][ NORMALISED ] );
      
      sprintf ( szTemp, _("\nCube decision rating:\t\t%-20s\t%-20s\n\n"),
                gettext ( aszRating[ rt [ 0 ] ] ), 
                gettext ( aszRating[ rt [ 1 ] ] ) );
      strcat ( szOutput, szTemp);

  }


  /* overall rating */

  if( psc->fMoves && psc->fCube ) {

    sprintf ( szTemp, _("\nOverall statistics:\n\n") );
    strcat ( szOutput, szTemp);

    if ( ms.nMatchTo ){
      sprintf ( szTemp,_("Error rate (total)\t\t"
                         "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n"
                         "Error rate (per decision)\t"
                         "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n"),
                -aaaar[ COMBINED ][ TOTAL ][ PLAYER_0 ][ NORMALISED ],
                -aaaar[ COMBINED ][ TOTAL ][ PLAYER_0 ][ UNNORMALISED ] * 100.0f,
                    
                -aaaar[ COMBINED ][ TOTAL ][ PLAYER_1 ][ NORMALISED ],
                -aaaar[ COMBINED ][ TOTAL ][ PLAYER_1 ][ UNNORMALISED ] * 100.0f,
                    
                -aaaar[ COMBINED ][ PERMOVE ][ PLAYER_0 ][ NORMALISED ],
                -aaaar[ COMBINED ][ PERMOVE ][ PLAYER_0 ][ UNNORMALISED ] * 100.0f,
                    
                -aaaar[ COMBINED ][ PERMOVE ][ PLAYER_1 ][ NORMALISED ],
                -aaaar[ COMBINED ][ PERMOVE ][ PLAYER_1 ][ UNNORMALISED ] * 100.0f
                );
      strcat ( szOutput, szTemp);
    } else {
      sprintf ( szTemp,_("Error rate (total)\t\t"
                         "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n"
                         "Error rate (per decision)\t"
                         "%+6.3f (%+7.3f%%)\t%+6.3f (%+7.3f%%)\n"),
                -aaaar[ COMBINED ][ TOTAL ][ PLAYER_0 ][ NORMALISED ],
                -aaaar[ COMBINED ][ TOTAL ][ PLAYER_0 ][ UNNORMALISED ],
                    
                -aaaar[ COMBINED ][ TOTAL ][ PLAYER_1 ][ NORMALISED ],
                -aaaar[ COMBINED ][ TOTAL ][ PLAYER_1 ][ UNNORMALISED ],
                    
                -aaaar[ COMBINED ][ PERMOVE ][ PLAYER_0 ][ NORMALISED ],
                -aaaar[ COMBINED ][ PERMOVE ][ PLAYER_0 ][ UNNORMALISED ],
                    
                -aaaar[ COMBINED ][ PERMOVE ][ PLAYER_1 ][ NORMALISED ],
                -aaaar[ COMBINED ][ PERMOVE ][ PLAYER_1 ][ UNNORMALISED ] 
                );
      strcat ( szOutput, szTemp);
    }

    for ( i = 0 ; i < 2; i++ )
      rt[ i ] = GetRating ( aaaar[ COMBINED ][ PERMOVE ][ i ][ NORMALISED ] );
      
    sprintf ( szTemp, _("Overall rating:\t\t\t%-20s\t%-20s\n\n"),
              gettext ( aszRating[ rt [ 0 ] ] ), 
              gettext ( aszRating[ rt [ 1 ] ] ) );
    strcat ( szOutput, szTemp);
  }

  /* calculate total error */

  if ( ms.nMatchTo ) {

    sprintf ( szTemp, 
              _("Match winning chance\nagainst opponent"
                ":\t\t%7.2f%%\t\t%7.2f%%\n"),
              100.0 * r, 100.0 * ( 1.0 - r ) );
    strcat ( szOutput, szTemp);

    sprintf ( szTemp, _("Guestimated abs. rating:\t%+7.2f\t\t%+7.2f\n\n"),
              absoluteFibsRating ( aaaar[ COMBINED ][ PERMOVE ][ PLAYER_0 ][ NORMALISED ], 
                                   ms.nMatchTo ),
              absoluteFibsRating ( aaaar[ COMBINED ][ PERMOVE ][ PLAYER_1 ][ NORMALISED ], 
                                   ms.nMatchTo ) );
    strcat ( szOutput, szTemp);



  }
  

}


extern void
CommandShowStatisticsMatch ( char *sz ) {

    char szOutput[4096];

    updateStatisticsMatch ( &lMatch );

#if USE_GTK
    if ( fX ) {
	GTKDumpStatcontext ( &scMatch, &ms, _("Statistics for all games") );
	return;
    }
#endif

    DumpStatcontext ( szOutput, &scMatch, _("Statistics for all games"));
    outputl(szOutput);
}


extern void
CommandShowStatisticsSession ( char *sz ) {

  CommandShowStatisticsMatch ( sz );

}


extern void
CommandShowStatisticsGame ( char *sz ) {

    movegameinfo *pmgi;
    char szOutput[4096];
    
    if( !plGame ) {
	outputl( _("No game is being played.") );
	return;
    }

    updateStatisticsGame ( plGame );

    pmgi = plGame->plNext->p;

    assert( pmgi->mt == MOVE_GAMEINFO );
    
#if USE_GTK
    if ( fX ) {
	GTKDumpStatcontext ( &pmgi->sc, &ms, _("Statistics for current game") );
	return;
    }
#endif

    DumpStatcontext ( szOutput, &pmgi->sc, _("Statistics for current game"));
    outputl( szOutput );
}


extern void CommandAnalyseMove ( char *sz ) {

  matchstate msx;

  if( ms.gs == GAME_NONE ) {
    outputl( _("No game in progress (type `new game' to start one).") );
    return;
  }
    

  if ( plLastMove && plLastMove->plNext && plLastMove->plNext->p ) {

    /* analyse move */

    memcpy ( &msx, &ms, sizeof ( matchstate ) );
    AnalyzeMove ( plLastMove->plNext->p, &msx, NULL, FALSE );

#if USE_GTK
  if( fX )
    GTKUpdateAnnotations();
#endif
  }
  else
    outputl ( _("Sorry, cannot analyse move!") );

}


static void
updateStatisticsMove ( moverecord *pmr, matchstate *pms, 
                       statcontext *psc ) {

  FixMatchState ( pms, pmr );

  switch ( pmr->mt ) {
  case MOVE_GAMEINFO:
    IniStatcontext ( psc );
    break;

  case MOVE_NORMAL:
    if( pmr->n.fPlayer != pms->fMove ) {
      SwapSides( pms->anBoard );
      pms->fMove = pmr->n.fPlayer;
    }
      
    updateStatcontext ( psc, pmr, pms );
    break;

  case MOVE_DOUBLE:
    if( pmr->d.fPlayer != pms->fMove ) {
      SwapSides( pms->anBoard );
      pms->fMove = pmr->d.fPlayer;
    }
      
      
    updateStatcontext ( psc, pmr, pms );
    break;

  case MOVE_TAKE:
  case MOVE_DROP:

    updateStatcontext ( psc, pmr, pms );
    break;

  default:
    break;

  }

  ApplyMoveRecord( pms, pmr );
  
  psc->fMoves = fAnalyseMove;
  psc->fCube = fAnalyseCube;
  psc->fDice = fAnalyseDice;
  
}



extern void
updateStatisticsGame ( list *plGame ) {

  list *pl;
  moverecord *pmr;
  movegameinfo *pmgi = plGame->plNext->p;
  matchstate msAnalyse;
  
  assert( pmgi->mt == MOVE_GAMEINFO );
    
  for( pl = plGame->plNext; pl != plGame; pl = pl->plNext ) {
    
    pmr = pl->p;
    
    updateStatisticsMove ( pmr, &msAnalyse, &pmgi->sc );

  }
    
}





extern void
updateStatisticsMatch ( list *plMatch ) {

  list *pl;
  movegameinfo *pmgi;

  if( ListEmpty( plMatch ) ) 
    /* no match in progress */
    return;

  IniStatcontext( &scMatch );
  
  for( pl = plMatch->plNext; pl != plMatch; pl = pl->plNext ) {
    
    updateStatisticsGame( pl->p );
    
    pmgi = ( (list *) pl->p )->plNext->p;
    assert( pmgi->mt == MOVE_GAMEINFO );
    AddStatcontext( &pmgi->sc, &scMatch );

  }

}



extern int getLuckRating ( const float rLuck ) {

  if ( rLuck < -0.10 )
    return 0;
  else if ( rLuck < -0.06 )
    return 1;
  else if ( rLuck < -0.02 )
    return 2;
  else if ( rLuck < +0.02 )
    return 3;
  else if ( rLuck < +0.06 )
    return 4;
  else
    return 5;

}
