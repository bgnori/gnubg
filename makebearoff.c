/*
 * makebearoff.c
 *
 * by Gary Wong <gary@cs.arizona.edu>, 1997-1999.
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
 * $Id: makebearoff.c,v 1.9 2002/10/28 21:09:00 thyssen Exp $
 */

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "positionid.h"
#include "getopt.h"

static unsigned int *ausRolls;
static double aaEquity[ 924 ][ 924 ];


static void
CalcIndex ( const unsigned short int aProb[ 32 ],
            unsigned int *piIdx, unsigned int *pnNonZero ) {

  int i;
  int j = 32;


  /* find max non-zero element */

  for ( i = 31; i >= 0; --i )
    if ( aProb[ i ] ) {
      j = i;
      break;
    }

  /* find min non-zero element */

  *piIdx = 0;
  for ( i = 0; i <= j; ++i ) 
    if ( aProb[ i ] ) {
      *piIdx = i;
      break;
    }


  *pnNonZero = j - *piIdx + 1;

}

static void BearOff( int nId, int nPoint, 
                     unsigned short aaProb[][ 32 ],
                     unsigned short aaGammonProb[][ 32 ] ) {

    int i, iBest, iMode, j, anRoll[ 2 ], anBoard[ 2 ][ 25 ],
      anBoardTemp[ 2 ][ 25 ], aProb[ 32 ], aGammonProb[ 32 ];
    movelist ml;
    int k;
    unsigned int us;

    PositionFromBearoff( anBoard[ 1 ], nId, nPoint );

    for( i = 0; i < 32; i++ )
	aProb[ i ] = aGammonProb[ i ] = 0;
    
    for( i = nPoint; i < 25; i++ )
	anBoard[ 1 ][ i ] = 0;

    for( i = 0; i < 25; i++ )
	anBoard[ 0 ][ i ] = 0;

    for ( i = 0, k = 0; i < nPoint; ++i )
      k += anBoard[ 1 ][ i ];

    if ( k < 15 )
      aGammonProb[ 0 ] = 0xFFFF * 36;
	    
    for( anRoll[ 0 ] = 1; anRoll[ 0 ] <= 6; anRoll[ 0 ]++ )
	for( anRoll[ 1 ] = 1; anRoll[ 1 ] <= anRoll[ 0 ]; anRoll[ 1 ]++ ) {
	    GenerateMoves( &ml, anBoard, anRoll[ 0 ], anRoll[ 1 ], FALSE );

	    us = 0xFFFF; iBest = -1;
	    
	    for( i = 0; i < ml.cMoves; i++ ) {
		PositionFromKey( anBoardTemp, ml.amMoves[ i ].auch );

		j = PositionBearoff( anBoardTemp[ 1 ], nPoint );

		assert( j >= 0 );
		assert( j < nId );
		
		if( ausRolls[ j ] < us ) {
		    iBest = j;
		    us = ausRolls[ j ];
		}
	    }

	    assert( iBest >= 0 );

	    if( anRoll[ 0 ] == anRoll[ 1 ] ) {
              for( i = 0; i < 31; i++ ) {
                aProb[ i + 1 ] += aaProb[ iBest ][ i ];
                if ( k == 15 )
                  aGammonProb[ i + 1 ] += aaGammonProb[ iBest ][ i ];
              }
            }
	    else {
              for( i = 0; i < 31; i++ ) {
                aProb[ i + 1 ] += ( aaProb[ iBest ][ i ] << 1 );
                if ( k == 15 )
                  aGammonProb[ i + 1 ] += aaGammonProb[ iBest ][ i ] << 1;
              }
            }
	}
    
    for( i = 0, j = 0, iMode = 0; i < 32; i++ ) {
	j += ( aaProb[ nId ][ i ] = ( aProb[ i ] + 18 ) / 36 );
	if( aaProb[ nId ][ i ] > aaProb[ nId ][ iMode ] )
	    iMode = i;
    }

    aaProb[ nId ][ iMode ] -= ( j - 0xFFFF );
    
    for( j = 0, i = 1; i < 32; i++ )
	j += i * aProb[ i ];

    ausRolls[ nId ] = j / 2359;

    /* gammon probs */

    for( i = 0, j = 0, iMode = 0; i < 32; i++ ) {
	j += ( aaGammonProb[ nId ][ i ] = ( aGammonProb[ i ] + 18 ) / 36 );
	if( aaGammonProb[ nId ][ i ] > aaGammonProb[ nId ][ iMode ] )
	    iMode = i;
    }

    aaGammonProb[ nId ][ iMode ] -= ( j - 0xFFFF );

}

static void BearOff2( int nUs, int nThem ) {

    int i, iBest, j, anRoll[ 2 ], anBoard[ 2 ][ 25 ],
	anBoardTemp[ 2 ][ 25 ];
    movelist ml;
    double r, rTotal;

    PositionFromBearoff( anBoard[ 0 ], nThem, 6 );
    PositionFromBearoff( anBoard[ 1 ], nUs, 6 );

    for( i = 6; i < 25; i++ )
	anBoard[ 1 ][ i ] = anBoard[ 0 ][ i ] = 0;

    rTotal = 0.0;
    
    for( anRoll[ 0 ] = 1; anRoll[ 0 ] <= 6; anRoll[ 0 ]++ )
	for( anRoll[ 1 ] = 1; anRoll[ 1 ] <= anRoll[ 0 ]; anRoll[ 1 ]++ ) {
	    GenerateMoves( &ml, anBoard, anRoll[ 0 ], anRoll[ 1 ], FALSE );

	    r = -1.0; iBest = -1;
	    
	    for( i = 0; i < ml.cMoves; i++ ) {
		PositionFromKey( anBoardTemp, ml.amMoves[ i ].auch );

		j = PositionBearoff( anBoardTemp[ 1 ], 6 );

		assert( j >= 0 );
		assert( j < nUs );

		if( 1.0 - aaEquity[ nThem ][ j ] > r ) {
		    iBest = j;
		    r = 1.0 - aaEquity[ nThem ][ j ];
		}
	    }

	    assert( iBest >= 0 );
	    
	    if( anRoll[ 0 ] == anRoll[ 1 ] )
		rTotal += r;
	    else
		rTotal += r * 2.0;
	}

    aaEquity[ nUs ][ nThem ] = rTotal / 36.0;
}

static void
generate ( const int nTSP, const int nTSC, 
           const int nOS, const int fMagicBytes,
           const int fCompress, const int fGammon ) {

    int i, j, k;
    int n;
    unsigned short int *pus;
    unsigned short int *pusGammon;
    unsigned int iIdx, nNonZero;

    unsigned int npos = 0;

#ifdef STDOUT_FILENO 
    FILE *output;

    if( !( output = fdopen( STDOUT_FILENO, "wb" ) ) ) {
	perror( "(stdout)" );
	return EXIT_FAILURE;
    }
#else
#define output stdout
#endif

    /* one sided database */

    if ( nOS ) {

      n = Combination ( nOS + 15, nOS );

      ausRolls = (unsigned int *) malloc ( sizeof ( unsigned int) * n );
      if ( ! ausRolls ) {
        fprintf ( stderr, "malloc failed for ausRolls!\n" );
        exit (3);
      }
        
      pus = (unsigned short int *) malloc ( sizeof ( unsigned short int ) * 
                                            n * 32 );
      if ( ! pus ) {
        fprintf ( stderr, "malloc failed for aaProb!\n" );
        exit (3);
      }

      pusGammon = 
        (unsigned short int *) malloc ( sizeof ( unsigned short int ) * 
                                        n * 32 );
      if ( ! pusGammon ) {
        fprintf ( stderr, "malloc failed for aaGammonProb!\n" );
        exit (3);
      }

      pus[ 0 ] = 0xFFFF;
      for( i = 1; i < 32; ++i )
        pus[ i ] = 0;

      pusGammon[ 0 ] = 0xFFFF;
      for( i = 1; i < 32; ++i )
        pusGammon[ i ] = 0;

      ausRolls[ 0 ] = 0;

      for( i = 1; i < n; i++ ) {
	BearOff( i, nOS, pus, pusGammon );

	if( !( i % 100 ) )
          fprintf( stderr, "1:%d/%d        \r", i, n );
      }

      if ( fMagicBytes ) {
        char sz[ 21 ];
        sprintf ( sz, "%02dx%1dxxxxxxxxxxxxxxx", nOS, fCompress );
        puts ( sz );
      }

      if ( fCompress ) {

        npos = 0;

        for ( i = 0; i < n; ++i ) {

          putc ( npos & 0xFF, output );
          putc ( ( npos >> 8 ) & 0xFF, output );
          putc ( ( npos >> 16 ) & 0xFF, output );
          putc ( ( npos >> 24 ) & 0xFF, output );
          
          CalcIndex ( pus + i *32, &iIdx, &nNonZero );

          putc ( nNonZero & 0xFF, output );

          putc ( iIdx & 0xFF, output );

          npos += nNonZero;

          CalcIndex ( pusGammon + i *32, &iIdx, &nNonZero );

          putc ( nNonZero & 0xFF, output );

          putc ( iIdx & 0xFF, output );

          npos += nNonZero;

        }

      }

      for( i = 0; i < n; i++ ) {

        if ( fCompress ) {
          CalcIndex ( pus + i * 32 , &iIdx, &nNonZero );
        }
        else {
          iIdx = 0;
          nNonZero = 32;
        }

	for( j = iIdx; j < iIdx + nNonZero; j++ ) {
          putc ( pus[ i * 32 + j ] & 0xFF, output );
          putc ( pus[ i * 32 + j ] >> 8, output );
        }

        if ( fGammon ) {
          
          if ( fCompress ) {
            CalcIndex ( pusGammon + i * 32, &iIdx, &nNonZero );
          }
          else {
            iIdx = 0;
            nNonZero = 32;
          }
          
          for( j = iIdx; j < iIdx + nNonZero; j++ ) {
            
            putc ( pusGammon[ i * 32 + j ] & 0xFF, output );
            putc ( pusGammon[ i * 32 + j ] >> 8, output );
          }
          
        }
        
      }
      
    }

    if ( nTSP && nTSC ) {

      /* two-sided database */

      for( i = 0; i < 924; i++ ) {
	aaEquity[ i ][ 0 ] = 0.0;
	aaEquity[ 0 ][ i ] = 1.0;
      }

      for( i = 1; i < 924; i++ ) {
	for( j = 0; j < i; j++ )
          BearOff2( i - j, j + 1 );

	fprintf( stderr, "2a:%d   \r", i );
      }

      for( i = 0; i < 924; i++ ) {
	for( j = i + 2; j < 924; j++ )
          BearOff2( i + 925 - j, j );

	fprintf( stderr, "2b:%d   \r", i );
      }

      for( i = 0; i < 924; i++ )
	for( j = 0; j < 924; j++ ) {
          k = aaEquity[ i ][ j ] * 65535.5;
	    
          putc( k & 0xFF, output );
          putc( k >> 8, output );
	}
    }    
}


static void
usage ( char *arg0 ) {

  printf ( "Usage: %s [options]\n"
           "Options:\n"
           "  -t, --two-sided PxC Number of points and number of chequers\n"
           "                      for two-sided database\n"
           "  -c, --compress      Use compression scheme "
                                  "for one-sided databases\n"
           "  -g, --gammons       Include gammon distribution for one-sided"
                                  " databases\n"
           "  -o, --one-sided P   Number of points for one-sided database\n"
           "  -m, --magic-bytes   Write magic bytes\n"
           "  -v, --version       Show version information and exit\n"
           "  -h, --help          Display usage and exit\n"
           "\n"
           "To generate gnubg.bd:\n"
           "%s -t 6x6 -o 6 > gnubg.bd\n"
           "\n",
           arg0, arg0 );

}

static void
version ( void ) {

  printf ( "makebearoff $Revision: 1.9 $\n" );

}


extern int main( int argc, char **argv ) {

  int nTSP = 6, nTSC = 6;
  int nOS = 6;
  int fMagicBytes = FALSE;
  char ch;
  int fCompress = FALSE;
  int fGammon = FALSE;

  static struct option ao[] = {
    { "two-sided", required_argument, NULL, 't' },
    { "one-sided", required_argument, NULL, 'o' },
    { "magic-bytes", no_argument, NULL, 'm' },
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'v' },
    { "compress", no_argument, NULL, 'c' },
    { "gammon", no_argument, NULL, 'g' },
    { NULL, 0, NULL, 0 }
  };

  while ( ( ch = getopt_long ( argc, argv, "t:o:mhvcg", ao, NULL ) ) !=
          (char) -1 ) {
    switch ( ch ) {
    case 't': /* size of two-sided */
      sscanf ( optarg, "%dx%d", &nTSP, &nTSC );
      break;
    case 'o': /* size of one-sided */
      nOS = atoi ( optarg );
      break;
    case 'm': /* magic bytes */
      fMagicBytes = TRUE;
      break;
    case 'h': /* help */
      usage ( argv[ 0 ] );
      exit ( 0 );
      break;
    case 'v': /* version */
      version ();
      exit ( 0 );
      break;
    case 'c': /* compress */
      fCompress = TRUE;
      break;
    case 'g': /* gammons */
      fGammon = TRUE;
      break;
    default:
      usage ( argv[ 0 ] );
      exit ( 1 );
    }
  }

  if ( nOS > 18 ) {
    fprintf ( stderr, 
              "Size of one-sided bearoff database must be between "
              "0 and 18\n" );
    exit ( 2 );
  }

  if ( nOS ) 
    fprintf ( stderr, 
              "One-sided database:\n"
              "Number of points   : %12d\n"
              "Number of positions: %'12d\n"
              "Size of file (unc) : %'12d bytes\n",
              nOS, 
              Combination ( nOS + 15, nOS ),
              ( fGammon ) ? 
              2 * Combination ( nOS + 15, nOS ) * 64 :
              Combination ( nOS + 15, nOS ) * 64 );
  else
    fprintf ( stderr, 
              "No one-sided database created.\n" );

  if ( nTSC && nTSP ) 
    fprintf ( stderr,
              "Two-sided database:\n"
              "Number of points   : %12d\n"
              "Number of chequers : %12d\n"
              "Number of positions: %'12d\n"
              "Size of file       : %'12d\n bytes",
              nTSP, nTSC, -1, -1 );
  else
    fprintf ( stderr, 
              "No two-sided database created.\n" );
      
  generate ( nTSP, nTSC, nOS, fMagicBytes, fCompress, fGammon );

  return 0;

}

