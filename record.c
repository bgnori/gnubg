/*
 * record.c
 *
 * by Gary Wong <gtw@gnu.org>, 2002.
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
 * $Id: record.c,v 1.3 2002/09/18 19:18:52 gtw Exp $
 */

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#if !HAVE_GETPID
#include <time.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "backgammon.h"
#if USE_GTK
#include "gtkgame.h"
#endif
#include "i18n.h"
#include "record.h"

static int anAvg[ NUM_AVG - 1 ] = { 20, 100, 500 };

/* exponential filter function, using the first three terms of the
   Taylor expansion of e^x */
#define DECAY(n) ( 1.0 - 1.0/(n) + 0.5/(n)/(n) )
static float arDecay[ NUM_AVG - 1 ] = {
    DECAY(20), DECAY(100), DECAY(500)
};

extern int RecordReadItem( FILE *pf, char *pch, playerrecord *ppr ) {

    int ch, i;
    expaverage ea;

    while( isspace( ch = getc( pf ) ) )
	;

    if( ch == EOF ) {
	if( feof( pf ) )
	    return 1;
	else {
	    outputerr( pch );
	    return -1;
	}
    }
    
    /* read and unescape name */
    i = 0;
    do {
	if( ch == EOF ) {
	    if( feof( pf ) )
		fprintf( stderr, "%s: invalid record file\n", pch );
	    else
		outputerr( pch );
	    return -1;
	} else if( ch == '\\' ) {
	    ppr->szName[ i ] = ( getc( pf ) & 007 ) << 6;
	    ppr->szName[ i ] |= ( getc( pf ) & 007 ) << 3;
	    ppr->szName[ i++ ] |= getc( pf ) & 007;
	} else
	    ppr->szName[ i++ ] = ch;
    } while( i < 31 && !isspace( ch = getc( pf ) ) );
    ppr->szName[ i ] = 0;
    
    fscanf( pf, " %d ", &ppr->cGames );
    if( ppr->cGames < 0 )
	ppr->cGames = 0;
    
    for( ea = 0; ea < NUM_AVG; ea++ )
	if( fscanf( pf, "%g %g %g %g ",
		    &ppr->arErrorChequerplay[ ea ],
		    &ppr->arErrorCube[ ea ],
		    &ppr->arErrorCombined[ ea ],
		    &ppr->arLuck[ ea ] ) < 4 ) {
	    if( ferror( pf ) )
		outputerr( pch );
	    else
		fprintf( stderr, "%s: invalid record file\n", pch );
	    
	    return -1;
	}

    return 0;
}

static int RecordWriteItem( FILE *pf, char *pch, playerrecord *ppr ) {

    char *pchName;
    expaverage ea;

    /* write escaped name */
    for( pchName = ppr->szName; *pchName; pchName++ )
	if( isalnum( *pchName ) ? ( putc( *pchName, pf ) == EOF ) :
	    ( fprintf( pf, "\\%03o", *pchName ) < 0 ) ) {
	    outputerr( pch );
	    return -1;
	}

    fprintf( pf, " %d ", ppr->cGames );
    for( ea = 0; ea < NUM_AVG; ea++ )
	fprintf( pf, "%g %g %g %g ",
		 ppr->arErrorChequerplay[ ea ],
		 ppr->arErrorCube[ ea ],
		 ppr->arErrorCombined[ ea ],
		 ppr->arLuck[ ea ] );

    putc( '\n', pf );

    if( ferror( pf ) ) {
	outputerr( pch );
	return -1;
    }
    
    return 0;
}

static int RecordRead( FILE **ppfOut, char **ppchOut, playerrecord apr[ 2 ] ) {

    int i, n;
    expaverage ea;
    playerrecord pr;
    FILE *pfIn;
#if __GNUC__
    char sz[ strlen( szHomeDirectory ) + 10 ];
#elif HAVE_ALLOCA
    char *sz = alloca( strlen( szHomeDirectory ) + 10 );
#else
    char sz[ 4096 ];
#endif

    *ppchOut = malloc( strlen( szHomeDirectory ) + 16 );
    sprintf( *ppchOut, "%s/.gnubgpr%06d", szHomeDirectory,
#if HAVE_GETPID
	     (int) getpid() % 1000000
#else
	     (int) time( NULL ) % 1000000
#endif
	);

    if( !( *ppfOut = fopen( *ppchOut, "w" ) ) ) {
	outputerr( *ppchOut );
	free( *ppchOut );
	return -1;
    }

    for( i = 0; i < 2; i++ ) {
	strcpy( apr[ i ].szName, ap[ i ].szName );
	apr[ i ].cGames = 0;
	for( ea = 0; ea < NUM_AVG; ea++ ) {
	    apr[ i ].arErrorChequerplay[ ea ] = 0.0;
	    apr[ i ].arErrorCube[ ea ] = 0.0;
	    apr[ i ].arErrorCombined[ ea ] = 0.0;
	    apr[ i ].arLuck[ ea ] = 0.0;
	}
    }
    
    sprintf( sz, "%s/.gnubgpr", szHomeDirectory );
    if( !( pfIn = fopen( sz, "r" ) ) )
	/* could not read existing records; assume empty */
	return 0;

    while( !( n = RecordReadItem( pfIn, sz, &pr ) ) )
	if( !CompareNames( pr.szName, apr[ 0 ].szName ) )
	    apr[ 0 ] = pr;
	else if( !CompareNames( pr.szName, apr[ 1 ].szName ) )
	    apr[ 1 ] = pr;
	else if( RecordWriteItem( *ppfOut, *ppchOut, &pr ) < 0 ) {
	    n = -1;
	    break;
	}

    fclose( pfIn );
    
    return n < 0 ? -1 : 0;
}

static int RecordWrite( FILE *pfOut, char *pchOut, playerrecord apr[ 2 ] ) {

#if __GNUC__
    char sz[ strlen( szHomeDirectory ) + 10 ];
#elif HAVE_ALLOCA
    char *sz = alloca( strlen( szHomeDirectory ) + 10 );
#else
    char sz[ 4096 ];
#endif

    if( RecordWriteItem( pfOut, pchOut, &apr[ 0 ] ) ||
	RecordWriteItem( pfOut, pchOut, &apr[ 1 ] ) ) {
	free( pchOut );
	return -1;
    }
    
    if( fclose( pfOut ) ) {
	outputerr( pchOut );
	free( pchOut );
	return -1;
    }
    
    sprintf( sz, "%s/.gnubgpr", szHomeDirectory );

    if( rename( pchOut, sz ) ) {
	outputerr( sz );
	free( pchOut );
	return -1;
    }

    free( pchOut );
    return 0;
}

static int RecordAbort( char *pchOut ) {

    remove( pchOut );
    free( pchOut );
    return 0;
}

static int RecordAddGame( list *plGame, playerrecord apr[ 2 ] ) {

    movegameinfo *pmgi = plGame->plNext->p;
    int i;
    expaverage ea;
    float aaaar[ 3 ][ 2 ][ 2 ][ 2 ];
    
    assert( pmgi->mt == MOVE_GAMEINFO );
    
    if( !pmgi->sc.fMoves || !pmgi->sc.fCube || !pmgi->sc.fDice )
	/* game is not completely analysed */
	return -1;

    for( i = 0; i < 2; i++ ) {
	apr[ i ].cGames++;

	getMWCFromError( &pmgi->sc, aaaar );
	
	apr[ i ].arErrorChequerplay[ EXPAVG_TOTAL ] =
	    ( apr[ i ].arErrorChequerplay[ EXPAVG_TOTAL ] *
	      ( apr[ i ].cGames - 1 ) +
		aaaar[ CHEQUERPLAY ][ PERMOVE ][ i ][ NORMALISED ] ) /
	    apr[ i ].cGames;
	apr[ i ].arErrorCube[ EXPAVG_TOTAL ] =
	    ( apr[ i ].arErrorCube[ EXPAVG_TOTAL ] *
	      ( apr[ i ].cGames - 1 ) +
		aaaar[ CUBEDECISION ][ PERMOVE ][ i ][ NORMALISED ] ) /
	    apr[ i ].cGames;
	apr[ i ].arErrorCombined[ EXPAVG_TOTAL ] =
	    ( apr[ i ].arErrorCombined[ EXPAVG_TOTAL ] *
	      ( apr[ i ].cGames - 1 ) +
		aaaar[ COMBINED ][ PERMOVE ][ i ][ NORMALISED ] ) /
	    apr[ i ].cGames;
	apr[ i ].arLuck[ EXPAVG_TOTAL ] =
	    ( apr[ i ].arLuck[ EXPAVG_TOTAL ] *
	      ( apr[ i ].cGames - 1 ) +
	      pmgi->sc.arLuck[ i ][ 0 ] ) / apr[ i ].cGames;

	for( ea = EXPAVG_20; ea < NUM_AVG; ea++ )
	    if( apr[ i ].cGames == anAvg[ ea - 1 ] ) {
		apr[ i ].arErrorChequerplay[ ea ] =
		    apr[ i ].arErrorChequerplay[ EXPAVG_TOTAL ];
		apr[ i ].arErrorCube[ ea ] =
		    apr[ i ].arErrorCube[ EXPAVG_TOTAL ];
		apr[ i ].arErrorCombined[ ea ] =
		    apr[ i ].arErrorCombined[ EXPAVG_TOTAL ];
		apr[ i ].arLuck[ ea ] =
		    apr[ i ].arLuck[ EXPAVG_TOTAL ];
	    } else if( apr[ i ].cGames > anAvg[ ea - 1 ] ) {
		apr[ i ].arErrorChequerplay[ ea ] =
		    apr[ i ].arErrorChequerplay[ ea ] * arDecay[ ea - 1 ] +
		    aaaar[ CHEQUERPLAY ][ PERMOVE ][ i ][ NORMALISED ] *
		    ( 1.0 - arDecay[ ea - 1 ] );
		apr[ i ].arErrorCube[ ea ] =
		    apr[ i ].arErrorCube[ ea ] * arDecay[ ea - 1 ] +
		    aaaar[ CUBEDECISION ][ PERMOVE ][ i ][ NORMALISED ] *
		    ( 1.0 - arDecay[ ea - 1 ] );
		apr[ i ].arErrorCombined[ ea ] =
		    apr[ i ].arErrorCombined[ ea ] * arDecay[ ea - 1 ] +
		    aaaar[ COMBINED ][ PERMOVE ][ i ][ NORMALISED ] *
		    ( 1.0 - arDecay[ ea - 1 ] );
		apr[ i ].arLuck[ ea ] =
		    apr[ i ].arLuck[ ea ] * arDecay[ ea - 1 ] +
		    pmgi->sc.arLuck[ i ][ 0 ] *
		    ( 1.0 - arDecay[ ea - 1 ] );
	    }
    }
    
    return 0;
}

extern void CommandRecordAddGame( char *sz ) {

    FILE *pf;
    char *pch;
    playerrecord apr[ 2 ];
    
    if( !plGame ) {
	outputl( _("No game is being played.") );
	return;
    }

    if( RecordRead( &pf, &pch, apr ) < 0 )
	return;
    
    if( RecordAddGame( plGame, apr ) < 0 ) {
	outputl( _("No game statistics are available.") );
	RecordAbort( pch );
	return;
    } else if( !RecordWrite( pf, pch, apr ) )
	outputl( _("Game statistics saved to player records.") );
}

extern void CommandRecordAddMatch( char *sz ) {
    
    list *pl;
    int c = 0;
    FILE *pf;
    char *pch;
    playerrecord apr[ 2 ];
   
    if( ListEmpty( &lMatch ) ) {
	outputl( _("No match is being played.") );
	return;
    }
    
    if( RecordRead( &pf, &pch, apr ) < 0 )
	return;

    for( pl = lMatch.plNext; pl != &lMatch; pl = pl->plNext )
	if( RecordAddGame( pl->p, apr ) >= 0 )
	    c++;

    if( !c ) {
	outputl( _("No game statistics are available.") );
	RecordAbort( pch );
	return;
    }
    
    if( RecordWrite( pf, pch, apr ) )
	return;

    if( c == 1 )
	outputl( _("Statistics from 1 game were saved to player records.") );
    else
	outputf( _("Statistics from %d games were saved to player "
		   "records.\n"), c );
}

extern void CommandRecordAddSession( char *sz ) {

    if( ListEmpty( &lMatch ) ) {
	outputl( _("No session is being played.") );
	return;
    }

    CommandRecordAddMatch( sz );
}

extern void CommandRecordShow( char *szPlayer ) {

    FILE *pfIn;
    int f = FALSE;
    playerrecord pr;
#if __GNUC__
    char sz[ strlen( szHomeDirectory ) + 10 ];
#elif HAVE_ALLOCA
    char *sz = alloca( strlen( szHomeDirectory ) + 10 );
#else
    char sz[ 4096 ];
#endif
    
    sprintf( sz, "%s/.gnubgpr", szHomeDirectory );
    if( !( pfIn = fopen( sz, "r" ) ) ) {
	outputerr( sz );
	return;
    }

#if USE_GTK
    if( fX )
	return GTKRecordShow( pfIn, sz, szPlayer );
#endif
    
    while( !RecordReadItem( pfIn, sz, &pr ) )
	if( !szPlayer || !*szPlayer || !CompareNames( szPlayer, pr.szName ) ) {
	    if( !f ) {
		outputl( _("                                Short-term  "
			   "Long-term   Total        Total\n"
			   "                                error rate  "
			   "error rate  error rate   luck\n"
			   "Name                            Cheq. Cube  "
			   "Cheq. Cube  Cheq. Cube   rate Games\n") );
		f = TRUE;
	    }
	    outputf( "%-31s %5.3f %5.3f %5.3f %5.3f %5.3f %5.3f %6.3f %4d\n",
		     pr.szName, pr.arErrorChequerplay[ EXPAVG_20 ],
		     pr.arErrorCube[ EXPAVG_20 ],
		     pr.arErrorChequerplay[ EXPAVG_100 ],
		     pr.arErrorCube[ EXPAVG_100 ],
		     pr.arErrorChequerplay[ EXPAVG_TOTAL ],
		     pr.arErrorCube[ EXPAVG_TOTAL ],
		     pr.arLuck[ EXPAVG_TOTAL ], pr.cGames );
	}

    if( ferror( pfIn ) )
	outputerr( sz );
    else if( !f ) {
	if( szPlayer && *szPlayer )
	    outputf( _("No record for player `%s' found.\n"), szPlayer );
	else
	    outputl( _("No player records found.") );
    }
    
    fclose( pfIn );
}
