/*
* matchequity.c
* by Joern Thyssen, 1999-2002
*
* Calculate matchequity table based on 
* formulas from [insert the Managent Science ref here].
*
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
* $Id: matchequity.c,v 1.20 2002/03/23 13:59:02 thyssen Exp $
*/

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "config.h"
#include "list.h"

#if HAVE_LIBXML2
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/catalog.h>
#endif

#if !HAVE_ERF
extern double erf( double x );
#endif

#define MAXCUBELEVEL  7
#define DELTA         0.08
#define DELTABAR      0.06
#define G1            0.25
#define G2            0.15
#define GAMMONRATE    0.25

#include "eval.h"
#include "matchequity.h"

typedef struct _parameter {

  char *szName, *szValue;

} parameter;


typedef struct _metparameters {

  char *szName;
  list lParameters;

} metparameters;

typedef struct _metdata {

  /* Data saved */

  /* Pre crawford MET */

  float aarMET[ MAXSCORE ][ MAXSCORE ];
  metparameters mpPreCrawford;

  /* post-crawford MET */

  float aarMETPostCrawford[ 2 ][ MAXSCORE ];
  metparameters ampPostCrawford[ 2 ];

  /* MET info */

  metinfo mi;

} metdata;

/*
* A1 (A2) is the match equity of player 1 (2)
* Btilde is the post-crawford match equities.
*/

float aafMET [ MAXSCORE ][ MAXSCORE ];
float aafMETPostCrawford[ 2 ][ MAXSCORE ];

metinfo miCurrent;


/*
 * Calculate area under normal distribution curve (with mean rMu and 
 * std.dev rSigma) from rMax to rMin.
 * 
 * Input:
 *   rMin, rMax: upper and lower integral limits
 *   rMu, rSigma: normal distribution parameters.
 *
 * Returns:
 *  area
 *
 */

static float
NormalDistArea ( float rMin, float rMax, float rMu, float rSigma ) {

  float rtMin, rtMax;
  float rInt1, rInt2;

  rtMin = ( rMin - rMu ) / rSigma;
  rtMax = ( rMax - rMu ) / rSigma;

  rInt1 = ( erf( rtMin / sqrt(2) ) + 1.0f ) / 2.0f;
  rInt2 = ( erf( rtMax / sqrt(2) ) + 1.0f ) / 2.0f;

  return rInt2 - rInt1;

}



int 
GetCubePrimeValue ( int i, int j, int nCubeValue ) {

  if ( (i < 2 * nCubeValue) && (j >= 2 * nCubeValue ) )

    /* automatic double */

    return 2*nCubeValue;
  else
    return nCubeValue;

}


/*
 * Calculate post-Crawford match equity table
 *
 * Formula taken from Zadeh, Management Science xx
 *
 * Input:
 *    rG: gammon rate for trailer
 *    rFD2: value of free drop at 1-away, 2-away
 *    rFD4: value of free drop at 1-away, 4-away
 *    iStart: score where met starts, e.g.
 *       iStart = 0 : 1-away, 1-away
 *       iStart = 15: 1-away, 16-away
 *    
 *   
 * Output:
 *    afMETPostCrawford: post-Crawford match equity table
 *
 */

void 
initPostCrawfordMET ( float afMETPostCrawford[ MAXSCORE ],
                      const int iStart,
                      const float rG,
                      const float rFD2, const float rFD4 ) {

  int i;

  /*
   * Calculate post-crawford match equities
   */

  for ( i = iStart; i < MAXSCORE; i++ ) {

    afMETPostCrawford[ i ] = 
      rG * 0.5 * 
      ( (i-4 >=0) ? afMETPostCrawford[ i-4 ] : 1.0 )
      + (1.0 - rG) * 0.5 * 
      ( (i-2 >=0) ? afMETPostCrawford[ i-2 ] : 1.0 );

    /*
     * add 1.5% at 1-away, 2-away for the free drop
     * add 0.4% at 1-away, 4-away for the free drop
     */

    if ( i == 1 )
      afMETPostCrawford[ i ] -= rFD2;

    if ( i == 3 )
      afMETPostCrawford[ i ] -= rFD4;

  }


}


/*
* Calculate match equity table from Zadeh's formula published
* in Management Science xx.xx,xx.
*
* Input:
*    rG1: gammon rate for leader of match
*    rG2: gammon rate for trailer of match
*    rDelta, rDeltaBar: parameters in the Zadeh model
*      (something to do with cube efficiencies)
*    rG3: post-Crawford gammon rate for trailer of match
*    afMETPostCrawford: post-Crawford match equity table
*
* Output:
*    aafMET: match equity table
*
*/

void
initMETZadeh ( float aafMET[ MAXSCORE ][ MAXSCORE ], 
               const float afMETPostCrawford[ MAXSCORE ],
               const float rG1, const float rG2, 
               const float rDelta, const float rDeltaBar ) {

  int i,j,k;
  int nCube;
  int nCubeValue, nCubePrimeValue;

/*
* D1bar (D2bar) is the equity of player 1 (2) at the drop point
* of player 2 (1) assuming dead cubes. 
* D1 (D2) is the equity of player 1 (2) at the drop point
* of player 2 (1) assuming semiefficient recubes.
*/

  float aaafD1 [ MAXSCORE ][ MAXSCORE ][ MAXCUBELEVEL ];
  float aaafD2 [ MAXSCORE ][ MAXSCORE ][ MAXCUBELEVEL ];
  float aaafD1bar [ MAXSCORE ][ MAXSCORE ][ MAXCUBELEVEL ];
  float aaafD2bar [ MAXSCORE ][ MAXSCORE ][ MAXCUBELEVEL ];


  /*
* Calculate 1-away,n-away match equities
*/

  for ( i = 0; i < MAXSCORE; i++ ) {

    aafMET[ i ][ 0 ] = 
      rG2 * 0.5 *
      ( (i-2 >=0) ? afMETPostCrawford[ i-2 ] : 1.0 )
      + (1.0 - rG2) * 0.5 * 
      ( (i-1 >=0) ? afMETPostCrawford[ i-1 ] : 1.0 );
    aafMET[ 0 ][ i ] = 1.0 - aafMET[ i ][ 0 ];

  }

  for ( i = 0; i < MAXSCORE ; i++ ) {

    for ( j = 0; j <= i ; j++ ) {

      for ( nCube = MAXCUBELEVEL-1; nCube >=0 ; nCube-- ) {

        nCubeValue = 1;
        for ( k = 0; k < nCube ; k++ )
          nCubeValue *= 2;

        /* Calculate D1bar */

        nCubePrimeValue = GetCubePrimeValue ( i, j, nCubeValue );

        aaafD1bar[ i ][ j ][ nCube ] = 
          ( GET_MET ( i - nCubeValue, j, aafMET )
            - rG2 * GET_MET ( i, j - 4 * nCubePrimeValue, aafMET )
            - (1.0-rG2) * GET_MET ( i, j - 2 * nCubePrimeValue, aafMET ) )
          /
          ( rG1 * GET_MET( i - 4 * nCubePrimeValue, j, aafMET )
            + (1.0-rG1) * GET_MET ( i - 2 * nCubePrimeValue, j, aafMET )
            - rG2 * GET_MET ( i, j - 4 * nCubePrimeValue, aafMET )
            - (1.0-rG2) * GET_MET ( i, j - 2 * nCubePrimeValue, aafMET ) );



        if ( i != j ) {

          nCubePrimeValue = GetCubePrimeValue ( j, i, nCubeValue );

          aaafD1bar[ j ][ i ][ nCube ] = 
            ( GET_MET ( j - nCubeValue, i, aafMET )
              - rG2 * GET_MET ( j, i - 4 * nCubePrimeValue, aafMET )
              - (1.0-rG2) * GET_MET ( j, i - 2 * nCubePrimeValue, aafMET ) )
            /
            ( rG1 * GET_MET( j - 4 * nCubePrimeValue, i, aafMET )
              + (1.0-rG1) * GET_MET ( j - 2 * nCubePrimeValue, i, aafMET )
              - rG2 * GET_MET ( j, i - 4 * nCubePrimeValue, aafMET )
              - (1.0-rG2) * GET_MET ( j, i - 2 * nCubePrimeValue, aafMET ) );


  
        }

        /* Calculate D2bar */

        nCubePrimeValue = GetCubePrimeValue ( j, i, nCubeValue );

        aaafD2bar[ i ][ j ][ nCube ] = 
          ( GET_MET ( j - nCubeValue, i, aafMET )
            - rG2 * GET_MET ( j, i - 4 * nCubePrimeValue, aafMET )
            - (1.0-rG2) * GET_MET ( j, i - 2 * nCubePrimeValue, aafMET ) )
          /
          ( rG1 * GET_MET( j - 4 * nCubePrimeValue, i, aafMET )
            + (1.0-rG1) * GET_MET ( j - 2 * nCubePrimeValue, i, aafMET )
            - rG2 * GET_MET ( j, i - 4 * nCubePrimeValue, aafMET )
            - (1.0-rG2) * GET_MET ( j, i - 2 * nCubePrimeValue, aafMET ) );


        if ( i != j ) {

          nCubePrimeValue = GetCubePrimeValue ( i, j, nCubeValue );
  
          aaafD2bar[ j ][ i ][ nCube ] = 
            ( GET_MET ( i - nCubeValue, j, aafMET )
              - rG2 * GET_MET ( i, j - 4 * nCubePrimeValue, aafMET )
              - (1.0-rG2) * GET_MET ( i, j - 2 * nCubePrimeValue, aafMET ) )
            /
            ( rG1 * GET_MET( i - 4 * nCubePrimeValue, j, aafMET )
              + (1.0-rG1) * GET_MET ( i - 2 * nCubePrimeValue, j, aafMET )
              - rG2 * GET_MET ( i, j - 4 * nCubePrimeValue, aafMET )
              - (1.0-rG2) * GET_MET ( i, j - 2 * nCubePrimeValue, aafMET ) );
  


        }

        /* Calculate D1 */

        if ( ( i < 2 * nCubeValue ) || ( j < 2 * nCubeValue ) ) {

          aaafD1[ i ][ j ][ nCube ] = aaafD1bar[ i ][ j ][ nCube ];
          if ( i != j )
            aaafD1[ j ][ i ][ nCube ] = aaafD1bar[ j ][ i ][ nCube ];

        } 
        else {
  
          aaafD1[ i ][ j ][ nCube ] = 
            1.0 + 
            ( aaafD2[ i ][ j ][ nCube+1 ] + rDelta )
            * ( aaafD1bar[ i ][ j ][ nCube ] - 1.0 );
          if ( i != j )
            aaafD1[ j ][ i ][ nCube ] = 
              1.0 + 
              ( aaafD2[ j ][ i ][ nCube+1 ] + rDelta )
              * ( aaafD1bar[ j ][ i ][ nCube ] - 1.0 );
  
        }


        /* Calculate D2 */

        if ( ( i < 2 * nCubeValue ) || ( j < 2 * nCubeValue ) ) {

          aaafD2[ i ][ j ][ nCube ] = aaafD2bar[ i ][ j ][ nCube ];
          if ( i != j )
            aaafD2[ j ][ i ][ nCube ] = aaafD2bar[ j ][ i ][ nCube ];

        } 
        else {
  
          aaafD2[ i ][ j ][ nCube ] = 
            1.0 + 
            ( aaafD1[ i ][ j ][ nCube+1 ] + rDelta )
            * ( aaafD2bar[ i ][ j ][ nCube ] - 1.0 );

          if ( i != j ) 
            aaafD2[ j ][ i ][ nCube ] = 
              1.0 + 
              ( aaafD1[ j ][ i ][ nCube+1 ] + rDelta )
              * ( aaafD2bar[ j ][ i ][ nCube ] - 1.0 );
        }


        /* Calculate A1 & A2 */

        if ( (nCube == 0) && ( i > 0 ) && ( j > 0 ) ) {

          aafMET[ i ][ j ] = 
            ( 
             ( aaafD2[ i ][ j ][ 0 ] + rDeltaBar - 0.5 )
             * GET_MET( i - 1, j, aafMET ) 
             + ( aaafD1[ i ][ j ][ 0 ] + rDeltaBar - 0.5 )
             * GET_MET( i, j - 1, aafMET ) )
            /
            (
             aaafD1[ i ][ j ][ 0 ] + rDeltaBar +
             aaafD2[ i ][ j ][ 0 ] + rDeltaBar - 1.0 );


          if ( i != j )
            aafMET[ j ][ i ] = 1.0 - aafMET[ i ][ j ];

        }

      }

    }

  }

}


extern int
GetPoints ( float arOutput [ 5 ], cubeinfo *pci, float arCP[ 2 ] ) {

  /*
   * Input:
   * - arOutput: we need the gammon and backgammon ratios
   *   (we assume arOutput is evaluate for pci -> fMove)
   * - anScore: the current score.
   * - nMatchTo: matchlength
   * - pci: value of cube, who's turn is it
   * 
   *
   * Output:
   * - arCP : cash points with live cube
   * These points are necesary for the linear
   * interpolation used in cubeless -> cubeful equity 
   * transformation.
   */

  /* Match play */

  /* normalize score */

  int i = pci->nMatchTo - pci->anScore[ 0 ] - 1;
  int j = pci->nMatchTo - pci->anScore[ 1 ] - 1;



  int nCube = pci -> nCube;

  float arCPLive [ 2 ][ MAXCUBELEVEL ];
  float arCPDead [ 2 ][ MAXCUBELEVEL ];
  float rG0, rG1, rBG0, rBG1;

  int nDead, n, nMax, nCubeValue, nCubePrimeValue;

  /* Gammon and backgammon ratio's. 
     Avoid division by zero in extreme cases. */

  if ( ! pci -> fMove ) {

    /* arOutput evaluated for player 0 */

    if ( arOutput[ OUTPUT_WIN ] > 0.0 ) {
      rG0 = ( arOutput[ OUTPUT_WINGAMMON ] -
              arOutput[ OUTPUT_WINBACKGAMMON ] ) /
        arOutput[ OUTPUT_WIN ];
      rBG0 = arOutput[ OUTPUT_WINBACKGAMMON ] /
        arOutput[ OUTPUT_WIN ];
    }
    else {
      rG0 = 0.0;
      rBG0 = 0.0;
    }

    if ( arOutput[ OUTPUT_WIN ] < 1.0 ) {
      rG1 = ( arOutput[ OUTPUT_LOSEGAMMON ] -
              arOutput[ OUTPUT_LOSEBACKGAMMON ] ) /
        ( 1.0 - arOutput[ OUTPUT_WIN ] );
      rBG1 = arOutput[ OUTPUT_LOSEBACKGAMMON ] /
        ( 1.0 - arOutput[ OUTPUT_WIN ] );
    }
    else {
      rG1 = 0.0;
      rBG1 = 0.0;
    }

  }
  else {

    /* arOutput evaluated for player 1 */

    if ( arOutput[ OUTPUT_WIN ] > 0.0 ) {
      rG1 = ( arOutput[ OUTPUT_WINGAMMON ] -
              arOutput[ OUTPUT_WINBACKGAMMON ] ) /
        arOutput[ OUTPUT_WIN ];
      rBG1 = arOutput[ OUTPUT_WINBACKGAMMON ] /
        arOutput[ OUTPUT_WIN ];
    }
    else {
      rG1 = 0.0;
      rBG1 = 0.0;
    }

    if ( arOutput[ OUTPUT_WIN ] < 1.0 ) {
      rG0 = ( arOutput[ OUTPUT_LOSEGAMMON ] -
              arOutput[ OUTPUT_LOSEBACKGAMMON ] ) /
        ( 1.0 - arOutput[ OUTPUT_WIN ] );
      rBG0 = arOutput[ OUTPUT_LOSEBACKGAMMON ] /
        ( 1.0 - arOutput[ OUTPUT_WIN ] );
    }
    else {
      rG0 = 0.0;
      rBG0 = 0.0;
    }
  }

  /* Find out what value the cube has when you or your
     opponent give a dead cube. */

  nDead = nCube;
  nMax = 0;

  while ( ( i >= 2 * nDead ) && ( j >= 2 * nDead ) ) {
    nMax ++;
    nDead *= 2;
  }

  for ( nCubeValue = nDead, n = nMax; 
        nCubeValue >= nCube; 
        nCubeValue >>= 1, n-- ) {

    /* Calculate dead and live cube cash points.
       See notes by me (Joern Thyssen) available from the
       'doc' directory.  (FIXME: write notes :-) ) */

    /* Even though it's a dead cube we take account of the opponents
       automatic redouble. */

    nCubePrimeValue = GetCubePrimeValue ( i, j, nCubeValue );

    /* Dead cube cash point for player 0 */

    arCPDead[ 0 ][ n ] = 
      ( ( 1.0 - rG1 - rBG1 ) * 
        GET_MET ( i, j - 2 * nCubePrimeValue, aafMET ) 
        + rG1 * GET_MET ( i, j - 4 * nCubePrimeValue, aafMET ) 
        + rBG1 * GET_MET ( i, j - 6 * nCubePrimeValue, aafMET ) 
        - GET_MET ( i - nCubeValue, j, aafMET ) )
      /
      ( ( 1.0 - rG1 - rBG1 ) * 
        GET_MET ( i, j - 2 * nCubePrimeValue, aafMET ) 
        + rG1 * GET_MET ( i, j - 4 * nCubePrimeValue, aafMET ) 
        + rBG1 * GET_MET ( i, j - 6 * nCubePrimeValue, aafMET ) 
        - ( 1.0 - rG0 - rBG0 ) *
        GET_MET( i - 2 * nCubePrimeValue, j, aafMET )
        - rG0 * GET_MET ( i - 4 * nCubePrimeValue, j, aafMET )
        - rBG0 * GET_MET ( i - 6 * nCubePrimeValue, j, aafMET ) );

    /* Dead cube cash point for player 1 */

    nCubePrimeValue = GetCubePrimeValue ( j, i, nCubeValue );

    arCPDead[ 1 ][ n ] = 
      ( ( 1.0 - rG0 - rBG0 ) * 
        GET_MET ( j, i - 2 * nCubePrimeValue, aafMET ) 
        + rG0 * GET_MET ( j, i - 4 * nCubePrimeValue, aafMET ) 
        + rBG0 * GET_MET ( j, i - 6 * nCubePrimeValue, aafMET ) 
        - GET_MET ( j - nCubeValue, i, aafMET ) )
      /
      ( ( 1.0 - rG0 - rBG0 ) * 
        GET_MET ( j, i - 2 * nCubePrimeValue, aafMET ) 
        + rG0 * GET_MET ( j, i - 4 * nCubePrimeValue, aafMET ) 
        + rBG0 * GET_MET ( j, i - 6 * nCubePrimeValue, aafMET ) 
        - ( 1.0 - rG1 - rBG1 ) *
        GET_MET( j - 2 * nCubePrimeValue, i, aafMET )
        - rG1 * GET_MET ( j - 4 * nCubePrimeValue, i, aafMET )
        - rBG1 * GET_MET ( j - 6 * nCubePrimeValue, i, aafMET ) );

    /* Live cube cash point for player 0 and 1 */

    if ( ( i < 2 * nCubeValue ) || ( j < 2 * nCubeValue ) ) {

      /* The doubled cube is going to be dead */

      arCPLive[ 0 ][ n ] = arCPDead[ 0 ][ n ];
      arCPLive[ 1 ][ n ] = arCPDead[ 1 ][ n ];

    } 
    else {

      /* Doubled cube is alive */

      arCPLive[ 0 ][ n ] = 1.0 - arCPLive[ 1 ][ n + 1] *
        ( GET_MET ( i - nCubeValue, j, aafMET )   
          - ( 1.0 - rG0 - rBG0 ) *
          GET_MET( i - 2 * nCubeValue, j, aafMET )
          - rG0 * GET_MET ( i - 4 * nCubeValue, j, aafMET )
          - rBG0 * GET_MET ( i - 6 * nCubeValue, j, aafMET ) )
        /
        ( GET_MET ( i, j - 2 * nCubeValue, aafMET )
          - ( 1.0 - rG0 - rBG0 ) *
          GET_MET( i - 2 * nCubeValue, j, aafMET )
          - rG0 * GET_MET ( i - 4 * nCubeValue, j, aafMET )
          - rBG0 * GET_MET ( i - 6 * nCubeValue, j, aafMET ) );

      arCPLive[ 1 ][ n ] = 1.0 - arCPLive[ 0 ][ n + 1 ] *
        ( GET_MET ( j - nCubeValue, i, aafMET )   
          - ( 1.0 - rG1 - rBG1 ) *
          GET_MET( j - 2 * nCubeValue, i, aafMET )
          - rG1 * GET_MET ( j - 4 * nCubeValue, i, aafMET )
          - rBG1 * GET_MET ( j - 6 * nCubeValue, i, aafMET ) )
        /
        ( GET_MET ( j, i - 2 * nCubeValue, aafMET )
          - ( 1.0 - rG1 - rBG1 ) *
          GET_MET( j - 2 * nCubeValue, i, aafMET )
          - rG1 * GET_MET ( j - 4 * nCubeValue, i, aafMET )
          - rBG1 * GET_MET ( j - 6 * nCubeValue, i, aafMET ) );

    }

  }

  /* return cash point for current cube level */

  arCP[ 0 ] = arCPLive[ 0 ][ 0 ];
  arCP[ 1 ] = arCPLive[ 1 ][ 0 ];

  /* debug output 
     for ( n = nMax; n >= 0; n-- ) {

     printf ("Cube %i\n"
     "Dead cube:    cash point 0 %6.3f\n"
     "              cash point 1 %6.3f\n"
     "Live cube:    cash point 0 %6.3f\n"
     "              cash point 1 %6.3f\n\n", 
     n,
     arCPDead[ 0 ][ n ], arCPDead[ 1 ][ n ],
     arCPLive[ 0 ][ n ], arCPLive[ 1 ][ n ] );

     }
  */

  return 0;

}



extern float
GetDoublePointDeadCube ( float arOutput [ 5 ], cubeinfo *pci ) {
  /*
   * Calculate double point for dead cubes
   */

  if ( ! pci->nMatchTo ) {

    /* Use Rick Janowski's formulas
       [insert proper reference here] */

    float rW, rL;

    if ( arOutput[ 0 ] > 0.0 )
      rW = 1.0 + ( arOutput[ 1 ] + arOutput[ 2 ] ) / arOutput[ 0 ];
    else
      rW = 1.0;

    if ( arOutput[ 0 ] < 1.0 )
      rL = 1.0 + ( arOutput[ 3 ] + arOutput[ 4 ] ) /
        ( 1.0 - arOutput [ 0 ] );
    else
      rL = 1.0;

    if ( pci->fCubeOwner == -1 && pci->fJacoby) {

      /* centered cube */

      if ( pci->fBeavers ) {
        return ( rL - 0.25 ) / ( rW + rL - 0.5 );
      }
      else {
        return ( rL - 0.5 ) / ( rW + rL - 1.0 );
      }
    }
    else {

      /* redoubles or Jacoby rule not in effect */

      return rL / ( rL + rW );
    }

  }
  else {

    /* Match play */

    /* normalize score */

    int i = pci->nMatchTo - pci->anScore[ pci->fMove ] - 1;
    int j = pci->nMatchTo - pci->anScore[ ! pci->fMove ] - 1;
    int nCube = pci->nCube;

    float rG1, rBG1, rG2, rBG2, rDTW, rNDW, rDTL, rNDL;
    float rRisk, rGain;

    /* FIXME: avoid division by zero */
    if ( arOutput[ OUTPUT_WIN ] > 0.0 ) {
      rG1 = ( arOutput[ OUTPUT_WINGAMMON ] -
              arOutput[ OUTPUT_WINBACKGAMMON ] ) /
        arOutput[ OUTPUT_WIN ];
      rBG1 = arOutput[ OUTPUT_WINBACKGAMMON ] /
        arOutput[ OUTPUT_WIN ];
    }
    else {
      rG1 = 0.0;
      rBG1 = 0.0;
    }

    if ( arOutput[ OUTPUT_WIN ] < 1.0 ) {
      rG2 = ( arOutput[ OUTPUT_LOSEGAMMON ] -
              arOutput[ OUTPUT_LOSEBACKGAMMON ] ) /
        ( 1.0 - arOutput[ OUTPUT_WIN ] );
      rBG2 = arOutput[ OUTPUT_LOSEBACKGAMMON ] /
        ( 1.0 - arOutput[ OUTPUT_WIN ] );
    }
    else {
      rG2 = 0.0;
      rBG2 = 0.0;
    }

    /* double point */

    /* FIXME: use the BG ratio */
    /* match equity for double, take; win */
    rDTW = (1.0 - rG1 - rBG1) * GET_MET ( i - 2 * nCube, j, aafMET ) +
      rG1 * GET_MET ( i - 4 * nCube, j, aafMET );
    rBG1 * GET_MET ( i - 6 * nCube, j, aafMET );

    /* match equity for no double; win */
    rNDW = (1.0 - rG1 - rBG1) * GET_MET ( i - nCube, j, aafMET ) +
      rG1 * GET_MET ( i - 2 * nCube, j, aafMET );
    rBG1 * GET_MET ( i - 3 * nCube, j, aafMET );

    /* match equity for double, take; loose */
    rDTL = (1.0 - rG2 - rBG2) * GET_MET ( i, j - 2 * nCube, aafMET ) +
      rG2 * GET_MET ( i, j - 4 * nCube, aafMET );
    rBG2 * GET_MET ( i, j - 6 * nCube, aafMET );

    /* match equity for double, take; loose */
    rNDL = (1.0 - rG2 - rBG2) * GET_MET ( i, j - nCube, aafMET ) +
      rG2 * GET_MET ( i, j - 2 * nCube, aafMET );
    rBG2 * GET_MET ( i, j - 3 * nCube, aafMET );

    /* risk & gain */

    rRisk = rNDL - rDTL;
    rGain = rDTW - rNDW;

    return rRisk / ( rRisk +  rGain );

  }

}



/* 
 * Extend match equity table to MAXSCORE using
 * David Montgomery's extrapolation algorithm.
 *
 * Input:
 *    nMaxScore: the length of the native met
 *    aarMET: the match equity table for scores below nMaxScore
 *
 * Output:
 *    aarMET: the match equity table for scores up to MAXSCORE.
 *
 */

static void
ExtendMET ( float aarMET[ MAXSCORE ][ MAXSCORE ],
            const int nMaxScore ) {

  static const float arStddevTable[] =
  { 0, 1.24, 1.27, 1.47, 1.50, 1.60, 1.61, 1.66, 1.68, 1.70, 1.72, 1.77 };

  float rStddev0, rStddev1, rGames, rSigma;
  int i,j;
  int nScore0, nScore1;

/* Extend match equity table */

  for ( i = nMaxScore; i < MAXSCORE; i++ ) {

    for ( j = 0; j <= i ; j++ ) {

      nScore0 = i + 1;
      nScore1 = j + 1;

      rGames = ( nScore0 + nScore1 ) / 2.00f;

      if ( nScore0 > 10 ) 
        rStddev0 = 1.77;
      else
        rStddev0 = arStddevTable[ nScore0 ];

      if ( nScore1 > 10 ) 
        rStddev1 = 1.77;
      else
        rStddev1 = arStddevTable[ nScore1 ];

      rSigma =
        sqrt ( rStddev0 * rStddev0 + rStddev1 * rStddev1 )
        * sqrt ( rGames );

      assert ( 6.0f * rSigma > nScore1 - nScore0 );

      aafMET[ i ][ j ] =
        1.0 - NormalDistArea ( nScore1 - nScore0, 6.0f * rSigma,
                               0.0f, rSigma );

    }
  }

  /* Generate j > i part of MET */

  for ( i = 0; i < MAXSCORE; i++ )
    for ( j = ( (i < nMaxScore) ? nMaxScore : i + 1);
          j < MAXSCORE; j++ )
      aafMET[ i ][ j ] = 1.0 - aafMET[ j ][ i ];

}


static void
initMP ( metparameters *pmp ) {

  pmp->szName = NULL;
  ListCreate ( &pmp->lParameters );

}


static void 
initMD ( metdata *pmd ) {

  int i;

  pmd->mi.szName = NULL;
  pmd->mi.szFileName = NULL;
  pmd->mi.szDescription = NULL;

  initMP ( &pmd->mpPreCrawford );

  for ( i = 0; i < 2; i++ )
    initMP ( &pmd->ampPostCrawford[ i ] );

}


extern void
freeP ( parameter *pp ) {

  if ( pp->szName )
    free ( pp->szName );
  if ( pp->szValue )
    free ( pp->szValue );
  free ( pp );

}


extern void
freeMP ( metparameters *pmp ) {

  list *pl;

  if ( pmp->szName )
    free ( pmp->szName );

  pl = &pmp->lParameters;

  while( pl->plNext != pl ) {

    freeP ( pl->plNext->p );
    ListDelete( pl->plNext );

  }

}


extern void
getDefaultMET ( metdata *pmd ) {

  int i, j;
  parameter *pp;

  static parameter apPreCrawford[] = 
  { { "gammon-rate-leader", "0.25" },
    { "gammon-rate-trailer", "0.15" },
    { "delta", "0.08" },
    { "deltebar", "0.06" } };

  static parameter apPostCrawford[] = 
  { { "gammon-rate-trailer", "0.25" },
    { "free-drop-2-away", "0.015" },
    { "free-drop-4-away", "0.004" } };

  /* Setup default met */

  initMD ( pmd );

  for ( i = 0; i < 4; i++ ) {
    pp = (parameter *) malloc ( sizeof ( parameter ) );
    pp->szName = strdup ( apPreCrawford[ i ].szName );
    pp->szValue = strdup ( apPreCrawford[ i ].szValue );
    ListInsert ( &pmd->mpPreCrawford.lParameters, pp );
  }

  for ( j = 0; j < 2; j++ ) {
    for ( i = 0; i < 3; i++ ) {
      pp = (parameter *) malloc ( sizeof ( parameter ) );
      pp->szName = strdup ( apPostCrawford[ i ].szName );
      pp->szValue = strdup ( apPostCrawford[ i ].szValue );
      ListInsert ( &pmd->ampPostCrawford[ j ].lParameters, pp );
    }
    pmd->ampPostCrawford[ j ].szName = strdup ( "zadeh" );
  }

  pmd->mpPreCrawford.szName = strdup ( "zadeh" );

  pmd->mi.szName = strdup ( "N. Zadeh, Management Science 23, 986 (1977)" );
  pmd->mi.szFileName = strdup ( "met/zadeh.xml" );
  pmd->mi.szDescription = strdup ( "" );
  pmd->mi.nLength = MAXSCORE;

}


static int
initMETFromParameters ( float aafMET [ MAXSCORE ][ MAXSCORE ],
                        float aafMETPostCrawford[ 2 ][ MAXSCORE ],
                        int nLength,
                        metparameters *pmp ) {

  if ( ! strcmp ( pmp->szName, "zadeh" ) ) {
    
    float rG1 = 0.25;
    float rG2 = 0.15;
    float rDelta = 0.08;
    float rDeltaBar = 0.06;
    list *pl; 
    parameter *pp;

    /* 
     * Use Zadeh's formulae 
     */

    /* obtain parameters */

    for ( pl = pmp->lParameters.plNext; pl != &pmp->lParameters; 
          pl = pl->plNext ) {

      pp = pl->p;

      if ( ! strcmp ( pp->szName, "gammon-rate-leader" ) )
        rG1 = atof ( pp->szValue );
      else if ( ! strcmp ( pp->szName, "gammon-rate-trailer" ) )
        rG2 = atof ( pp->szValue );
      else if ( ! strcmp ( pp->szName, "delta" ) )
        rDelta = atof ( pp->szValue );
      else if ( ! strcmp ( pp->szName, "delta-bar" ) )
        rDeltaBar = atof ( pp->szValue );

    }
    

    /* calculate table */

    initMETZadeh ( aafMET, aafMETPostCrawford[ 0 ], 
                   rG1, rG2, rDelta, rDeltaBar );
    return 0;

  }
  else {

    /* unknown type */

    return -1;

  }    
  

}


static int
initPostCrawfordMETFromParameters ( float afMETPostCrawford[ MAXSCORE ],
                                    int nLength,
                                    metparameters *pmp ) {

  if ( ! strcmp ( pmp->szName, "zadeh" ) ) {
    
    float rG = 0.25;
    float rFD2 = 0.015;
    float rFD4 = 0.004;
    list *pl;
    parameter *pp;

    /* 
     * Use Zadeh's formulae 
     */

    /* obtain parameters */

    for ( pl = pmp->lParameters.plNext; pl != &pmp->lParameters; 
          pl = pl->plNext ) {

      pp = pl->p;

      if ( ! strcmp ( pp->szName, "gammon-rate-trailer" ) )
        rG = atof ( pp->szValue );
      else if ( ! strcmp ( pp->szName, "free-drop-2-away" ) )
        rFD2 = atof ( pp->szValue );
      else if ( ! strcmp ( pp->szName, "free-drop-4-away" ) )
        rFD4 = atof ( pp->szValue );

    }

    /* calculate table */

    initPostCrawfordMET ( afMETPostCrawford, 0, rG, rFD2, rFD4 );
    return 0;

  }
  else {

    /* unknown type */

    return -1;

  }    
  

}


#if HAVE_LIBXML2

static void
parseRow ( float arRow[], xmlDocPtr doc, xmlNodePtr root ) {

  int iCol;
  xmlNodePtr cur;

  iCol = 0;

  for ( cur = root->xmlChildrenNode; cur; cur = cur->next ) {

    if ( ! strcmp ( cur->name, "me" ) ) {
      arRow[ iCol ]  = 
        atof ( xmlNodeListGetString ( doc, cur->xmlChildrenNode, 1 ) );

      iCol++;

    }

  }

}


static void
parsePreCrawfordExplicit ( metdata *pmd, xmlDocPtr doc, xmlNodePtr root ) {

  xmlNodePtr cur;
  int iRow;

  iRow = 0;

  for ( cur = root->xmlChildrenNode; cur; cur = cur->next ) {

    if ( ! strcmp ( cur->name, "row" ) )
      parseRow ( pmd->aarMET[ iRow++ ], doc, cur );

  }

}

static void
parseParameters ( list *plList, xmlDocPtr doc, xmlNodePtr root ) {

  xmlNodePtr cur;
  char *pc;
  parameter *pp;

  for ( cur = root->xmlChildrenNode; cur; cur = cur->next ) {

    if ( ! strcmp ( cur->name, "parameter" ) ) {

      pp = (parameter *) malloc ( sizeof ( parameter ) );

      pc = xmlGetProp ( cur, "name" );
      pp->szName = strdup ( pc ? pc : "" );
      
      pc = xmlNodeListGetString ( doc, cur->xmlChildrenNode, 1 );
      pp->szValue = strdup ( pc ? pc : "" );

      ListInsert ( plList, pp );

    }

  }


}


static void
parsePreCrawfordFormula ( metdata *pmd, xmlDocPtr doc, xmlNodePtr root ) {

  xmlNodePtr cur;

  for ( cur = root->xmlChildrenNode; cur; cur = cur->next ) {

    if ( ! strcmp ( cur->name, "parameters" ) )
      parseParameters ( &pmd->mpPreCrawford.lParameters, 
                        doc, cur );

  }

}


static void
parsePostCrawfordFormula ( metdata *pmd, xmlDocPtr doc, xmlNodePtr root,
                           const int fPlayer ) {

  xmlNodePtr cur;

  for ( cur = root->xmlChildrenNode; cur; cur = cur->next ) {

    if ( ! strcmp ( cur->name, "parameters" ) ) {

      if ( fPlayer < 0 ) {
        parseParameters ( &pmd->ampPostCrawford[ 0 ].lParameters, 
                          doc, cur );
        parseParameters ( &pmd->ampPostCrawford[ 1 ].lParameters, 
                          doc, cur );
      }
      else
        parseParameters ( &pmd->ampPostCrawford[ fPlayer ].lParameters, 
                          doc, cur );
    }

  }

}



static void
parsePostCrawfordExplicit ( metdata *pmd, xmlDocPtr doc, xmlNodePtr root,
                            const int fPlayer ) {

  xmlNodePtr cur;

  for ( cur = root->xmlChildrenNode; cur; cur = cur->next ) {

    if ( ! strcmp ( cur->name, "row" ) ) {

      if ( fPlayer < 0 ) {
        parseRow ( pmd->aarMETPostCrawford[ 0 ], doc, cur );
        parseRow ( pmd->aarMETPostCrawford[ 1 ], doc, cur );
      }
      else
        parseRow ( pmd->aarMETPostCrawford[ fPlayer ], doc, cur );

    }

  }

}




static void parsePreCrawford ( metdata *pmd, xmlDocPtr doc, xmlNodePtr root ) {

  if ( ! strcmp ( pmd->mpPreCrawford.szName, "explicit" ) )
    parsePreCrawfordExplicit ( pmd, doc, root );
  else
    parsePreCrawfordFormula ( pmd, doc, root );

}


static void parsePostCrawford ( metdata *pmd, xmlDocPtr doc, xmlNodePtr root,
                                const int fPlayer ) {

  int i = ( fPlayer < 0 ) ? 0 : fPlayer;

  if ( ! strcmp ( pmd->ampPostCrawford[ i ].szName, "explicit" ) )
    parsePostCrawfordExplicit ( pmd, doc, root, fPlayer );
  else
    parsePostCrawfordFormula ( pmd, doc, root, fPlayer );

}


static void parseInfo ( metdata *pmd, xmlDocPtr doc, xmlNodePtr root ) {

  xmlNodePtr cur;
  char *pc;

  for ( cur = root->xmlChildrenNode; cur; cur = cur->next ) {

    if ( ! strcmp ( cur->name, "name" ) ) {
      pc = xmlNodeListGetString ( doc, cur->xmlChildrenNode, 1 );
      pmd->mi.szName = ( pc ) ? strdup ( pc ) : NULL;
    }
    else if ( ! strcmp ( cur->name, "description" ) ) {
      pc = xmlNodeListGetString ( doc, cur->xmlChildrenNode, 1 );
      pmd->mi.szDescription = ( pc ) ? strdup ( pc ) : NULL;
    }
    else if ( ! strcmp ( cur->name, "length" ) )
      pmd->mi.nLength =
        atoi ( xmlNodeListGetString ( doc, cur->xmlChildrenNode, 1 ) );

  }

}

void validateWarning ( void *ctx,
                        const char *msg, 
                        ... ) {

  va_list ap;
  
  va_start ( ap, msg );
  vprintf ( msg, ap );
  va_end ( ap );

}

void validateError ( void *ctx,
                     const char *msg, 
                     ... ) {

  va_list ap;

  va_start ( ap, msg );
  vprintf ( msg, ap );
  va_end ( ap );

}


static int readMET ( metdata *pmd, const char *szFileName,
		     const char *szDir ) {

  xmlDocPtr doc;
  xmlNodePtr root, cur;

  xmlValidCtxt ctxt;

  char *pc, *pch;

  int fError;

  fError = 0;

  /* parse document */

  doc = xmlParseFile( pch = PathSearch( szFileName, szDir ) );
  free( pch );
  
  /* check root */

  root = xmlDocGetRootElement ( doc );

  if ( ! root ) {

    printf ( "Error reading XML file (%s): no document root!\n", szFileName );
    fError = 1;
    goto finish;

  }

  /* check if the dtd has the correct name */

  if ( xmlStrcmp ( root->name, (const xmlChar *) "met" ) ) {

    printf ( "Error reading XML file (%s): wrong DTD (%s)\n", 
             szFileName, root->name );
    fError = 1;
    goto finish;

  }

  /* 
   * validate document 
   */

  /* load catalogs */

  /* FIXME: any tweaks needed for win32? */

  xmlLoadCatalog( pch = PathSearch( "met/catalog", szDir ) );
  free( pch );
  xmlLoadCatalog ( PKGDATADIR "/met/catalog " );
  if ( ( pc = getenv ( "SGML_CATALOG_FILES" ) ) )
    xmlLoadCatalog ( pc );
  xmlLoadCatalog ( "./met/catalog" );
  xmlLoadCatalog ( "./catalog" );

  pc = xmlCatalogResolve ( doc->intSubset->ExternalID, 
                           doc->intSubset->SystemID );
  fError = ! pc;
  free ( pc );

  if ( fError ) {

    printf ( "met DTD not found\n" );
    fError = 1;
    goto finish;

  }

  ctxt.error = validateError;
  ctxt.warning = validateWarning;

  if ( ! xmlValidateDocument ( &ctxt, doc ) ) {

    printf ( "XML does not validate!\n" );
    fError = 1;
    goto finish;

  }

  /* initialise data */

  initMD ( pmd );

  /* fetch information from xml doc */

  for ( cur = root->xmlChildrenNode; cur; cur = cur->next ) {

    if ( ! strcmp ( cur->name, "info" ) )
      parseInfo ( pmd, doc, cur );
    else if ( ! strcmp ( cur->name, "pre-crawford-table" ) ) {

      pc = xmlGetProp ( cur, "type" );
      if ( pc )
        pmd->mpPreCrawford.szName = pc;
      else
        pmd->mpPreCrawford.szName = strdup ( "explicit" );

      parsePreCrawford ( pmd, doc, cur );

    }
    else if ( ! strcmp ( cur->name, "post-crawford-table" ) ) {

      int fPlayer;
      int i;

      pc = xmlGetProp ( cur, "player" );
      if ( pc ) {
        
        /* attribute "player" was specified */

        if ( ! strcmp ( pc, "both" ) || ! strcmp ( pc, "all" ) )
          fPlayer = -1;
        else
          fPlayer = atoi ( pc );

        if ( fPlayer < -1 || fPlayer > 1 ) {
          printf ( "Illegal value for attribute player: '%s'\n", pc );
          fError = 1;
          goto finish;
        }

      }
      else
        /* assume "both" */
        fPlayer = -1;


      for ( i = 0; i < 2; i++ ) {

        if ( fPlayer < 0 || i == fPlayer ) {

          pc = xmlGetProp ( cur, "type" );

          if ( pc )
            pmd->ampPostCrawford[ i ].szName = pc;
          else
            pmd->ampPostCrawford[ i ].szName = strdup ( "explicit" );
        }

      }

      parsePostCrawford ( pmd, doc, cur, fPlayer );

    }

  }

  if ( pmd->mi.nLength < 0 ) pmd->mi.nLength = MAXSCORE;

#ifdef UNDEF

  /* debug dump */

  printf ( "Name          : '%s'\n", pmd->mi.szName );
  printf ( "Description   : '%s'\n", pmd->mi.szDescription );
  printf ( "Length        : %d\n", pmd->mi.nLength );
  printf ( "pre-Crawford:\n" );

  if ( ! strcmp ( pmd->mpPreCrawford.szName, "explicit" ) ) {
    for ( j = 0; j < pmd->mi.nLength; j++ ) {
      printf ( "row %2d: ", j );
      for ( i = 0; i < pmd->mi.nLength; i++ ) {
        printf ( "%5.3f ", pmd->aarMET[ j ][ i ] );
      }
      printf ( "\n" );
    }
  }

  printf ( "post-Crawford:\n" );

  if ( ! strcmp ( pmd->mpPostCrawford.szName, "explicit" ) ) {
    printf ( "row %2d: ", 0 );
    for ( i = 0; i < pmd->mi.nLength; i++ ) {
      printf ( "%5.3f ", pmd->arMETPostCrawford[ i ] );
    }
    printf ( "\n" );
  }

#endif

 finish:

  if ( doc )
    xmlFreeDoc ( doc );

  if ( ! fError )
    pmd->mi.szFileName = strdup ( szFileName );

  return fError;

  
  
}

#endif

extern void
InitMatchEquity ( const char *szFileName, const char *szDir ) {

  int i,j;
  metdata md;

#ifdef HAVE_LIBXML2
  static int fTableLoaded = FALSE;

  /*
   * Read match equity table from XML file
   */

  if ( readMET ( &md, szFileName, szDir ) ) {

    if ( ! fTableLoaded ) {

      /* must read met */

      getDefaultMET ( &md );

    }
    else {

      fprintf ( stderr, "Error reading MET\n" );
      return;

    }
    
  }

  fTableLoaded = TRUE;

#else

  printf ( "Your version of GNU Backgammon does not support\n"
           "reading match equity files.\n"
           "Fallback to default Zadeh table.\n" );

  getDefaultMET ( &md );

#endif

  /*
   * Copy met to current met
   * Extend met (if needed)
   */

  /* post-Crawford table */

  for ( j = 0; j < 2; j++ ) {

    if ( ! strcmp ( md.ampPostCrawford[ j ].szName, "explicit" ) ) {

      /* copy and extend table */

      /* FIXME: implement better extension of post-Crawford table */

      for ( i = 0; i < md.mi.nLength; i++ )
        aafMETPostCrawford[ j ][ i ] = md.aarMETPostCrawford[ j ][ i ];

      initPostCrawfordMET ( aafMETPostCrawford[ j ], md.mi.nLength, 
                            GAMMONRATE, 0.015, 0.004 );

    }
    else {

      /* generate match equity table using Zadeh's formula */

      if ( initPostCrawfordMETFromParameters ( aafMETPostCrawford[ j ], 
                                               md.mi.nLength,
                                               &md.ampPostCrawford[ j ] ) < 0 ) {

        fprintf ( stderr, "Error generating post-Crawford MET\n" );
        return;

      }

    }

  }

  /* pre-Crawford table */

  if ( ! strcmp ( md.mpPreCrawford.szName, "explicit" ) ) {

    /* copy table */

    for ( i = 0; i < md.mi.nLength; i++ )
      for ( j = 0; j < md.mi.nLength; j++ )
        aafMET[ i ][ j ] = md.aarMET[ i ][ j ];

  }
  else {

    /* generate matc equity table using Zadeh's formula */

    if ( initMETFromParameters ( aafMET, aafMETPostCrawford,
                                 md.mi.nLength,
                                 &md.mpPreCrawford ) < 0 ) {

      fprintf ( stderr, "Error generating pre-Crawford MET\n" );
      return;

    }

  }

  /* Extend match equity table */

  ExtendMET ( aafMET, md.mi.nLength );


  /* garbage collect */

  freeMP ( &md.mpPreCrawford );

  for ( i = 0; i < 2; i++ )
    freeMP ( &md.ampPostCrawford[ i ] );

  if ( miCurrent.szName ) free ( miCurrent.szName );
  if ( miCurrent.szFileName ) free ( miCurrent.szFileName );
  if ( miCurrent.szDescription ) free ( miCurrent.szDescription );

  /* save match equity table information */

  memcpy ( &miCurrent, &md.mi, sizeof ( metinfo ) );

}




