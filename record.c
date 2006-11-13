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
 * $Id: record.c,v 1.20 2006/11/13 21:35:01 c_anthon Exp $
 */

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !HAVE_GETPID
#include <time.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>

#include "backgammon.h"
#if USE_GTK
#include "gtkgame.h"
#endif
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include "record.h"

static int anAvg[ NUM_AVG - 1 ] = { 20, 100, 500 };

/* exponential filter function, using the first three terms of the
   Taylor expansion of e^x */
#define DECAY(n) ( 1.0f - 1.0f/(n) + 0.5f/(n)/(n) )
static float arDecay[ NUM_AVG - 1 ] = {
    DECAY(20), DECAY(100), DECAY(500)
};

static int nVersion; /* file format when reading player record file */
/* 0: unknown format */
/* 1: original format (locale dependent) */
/* 2: locale independent format, with version header */

extern int RecordReadItem( FILE *pf, char *pch, playerrecord *ppr ) {

    int ch, i;
    expaverage ea;
    char ach[ 80 ];

 reread:
    while( isspace( ch = getc( pf ) ) )
	;

    if( ch == EOF ) {
	nVersion = 0;
	
	if( feof( pf ) )
	    return 1;
	else {
	    outputerr( pch );
	    return -1;
	}
    }

    if( !nVersion ) {
	/* we don't know which format we're parsing yet */
	fgets( ach, 80, pf );

	if( ch != '#' || strncmp( ach, " %Version:", 10 ) ) {
	    /* no version header; must be version 1 */
	    nVersion = 1;
	    rewind( pf );
	} else
	    if( ( nVersion = atoi( ach + 10 ) ) < 2 ) {
		nVersion = 0;
		outputerrf( _("%s: invalid record file version"), pch );
		return -1;
	    }
	goto reread;
    }

    /* read and unescape name */
    i = 0;
    do {
	if( ch == EOF ) {
	    if( feof( pf ) )
		outputerrf( _("%s: invalid record file"), pch );
	    else
		outputerr( pch );

	    nVersion = 0;
	    
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
    
    for( ea = 0; ea < NUM_AVG; ea++ ){
	    /* Not really cute, but let me guess that this will
	     * be cleaned out anytime in the near future. */
	gchar str1[G_ASCII_DTOSTR_BUF_SIZE];
	gchar str2[G_ASCII_DTOSTR_BUF_SIZE];
	gchar str3[G_ASCII_DTOSTR_BUF_SIZE];
	gchar str4[G_ASCII_DTOSTR_BUF_SIZE];
	
	if( fscanf( pf, "%s %s %s %s ", str1, str2, str3, str4) < 4){
	    if( ferror( pf ) )
		outputerr( pch );
	    else
		outputerrf( _("%s: invalid record file"), pch );

	    nVersion = 0;
    
	    return -1;
	}
        ppr->arErrorChequerplay[ ea ] = g_ascii_strtod(str1, NULL);
        ppr->arErrorCube[ ea ] =g_ascii_strtod(str2, NULL);
        ppr->arErrorCombined[ ea ] = g_ascii_strtod(str3, NULL);
        ppr->arLuck[ ea ]  = g_ascii_strtod(str4, NULL);
    }

    return 0;
}

static int RecordWriteItem( FILE *pf, char *pch, playerrecord *ppr ) {

    char *pchName;
    gchar buf0[G_ASCII_DTOSTR_BUF_SIZE];
    gchar buf1[G_ASCII_DTOSTR_BUF_SIZE];
    gchar buf2[G_ASCII_DTOSTR_BUF_SIZE];
    gchar buf3[G_ASCII_DTOSTR_BUF_SIZE];
    expaverage ea;

    if( !*ppr->szName )
	return 0;
    
    /* write escaped name */
    for( pchName = ppr->szName; *pchName; pchName++ )
	if( isalnum( *pchName ) ? ( putc( *pchName, pf ) == EOF ) :
	    ( fprintf( pf, "\\%03o", *pchName ) < 0 ) ) {
	    outputerr( pch );
	    return -1;
	}

    fprintf( pf, " %d ", ppr->cGames );
    for( ea = 0; ea < NUM_AVG; ea++ )
	fprintf( pf, "%s %s %s %s ",
		 g_ascii_formatd ( buf0, G_ASCII_DTOSTR_BUF_SIZE, "%g", ppr->arErrorChequerplay[ ea ]),
		 g_ascii_formatd ( buf1, G_ASCII_DTOSTR_BUF_SIZE, "%g", ppr->arErrorCube[ ea ]),
		 g_ascii_formatd ( buf2, G_ASCII_DTOSTR_BUF_SIZE, "%g", ppr->arErrorCombined[ ea ]),
		 g_ascii_formatd ( buf3, G_ASCII_DTOSTR_BUF_SIZE, "%g", ppr->arLuck[ ea ] ) );

    putc( '\n', pf );

    if( ferror( pf ) ) {
	outputerr( pch );
	return -1;
    }
    
    return 0;
}

static int RecordRead( FILE **ppfOut, char **ppchOut, playerrecord apr[ 2 ] ) {

    int n;
    int tmpfile;
    playerrecord pr;
    FILE *pfIn;
    char *sz = g_build_filename(szHomeDirectory, GNUBGPR, NULL);
    if ( !( tmpfile = g_file_open_tmp("gnubgprXXXXXX", ppchOut, NULL) ) || ! (*ppfOut = fdopen( tmpfile, "w" ))) {
	outputerr( *ppchOut );
	free( *ppchOut );
	return -1;
    }

    if( fputs( "# %Version: 2 ($Revision: 1.20 $)\n", *ppfOut ) < 0 ) {
	outputerr( *ppchOut );
	free( *ppchOut );
	return -1;
    }
    
    if( !( pfIn = g_fopen( sz, "r" ) ) ) {
        g_free(sz);
	/* could not read existing records; assume empty */
	return 0;
    }

    while( !( n = RecordReadItem( pfIn, sz, &pr ) ) )
	if( !CompareNames( pr.szName, apr[ 0 ].szName ) )
	    apr[ 0 ] = pr;
	else if( !CompareNames( pr.szName, apr[ 1 ].szName ) )
	    apr[ 1 ] = pr;
	else if( RecordWriteItem( *ppfOut, *ppchOut, &pr ) < 0 ) {
	    n = -1;
	    break;
	}
    g_free(sz);

    fclose( pfIn );
    
    return n < 0 ? -1 : 0;
}

static int RecordWrite( FILE *pfOut, char *pchOut, playerrecord apr[ 2 ] ) {

    char *sz = g_build_filename(szHomeDirectory, GNUBGPR, NULL);

    if( RecordWriteItem( pfOut, pchOut, &apr[ 0 ] ) ||
	RecordWriteItem( pfOut, pchOut, &apr[ 1 ] ) ) {
	free( pchOut );
	return -1;
    }
    
    if( fclose( pfOut ) ) {
	outputerr( pchOut );
	g_free( pchOut );
        g_free( sz );
	return -1;
    }
    
    if( g_rename( pchOut, sz ) ) {
            if ( g_unlink ( sz ) && errno != ENOENT ) {
                    /* do not complain if file is not found */
                    outputerr ( sz );
                    g_free ( pchOut );
                    g_free( sz );
                    return -1;
            }
            int c;                    
            FILE *IPFile, *OPFile;
            IPFile = fopen(pchOut,"r");
            OPFile = fopen(sz,"w");
            while ((c = fgetc(IPFile)) != EOF)
                    fputc(c, OPFile);

            fclose(IPFile);
            fclose(OPFile);
            free( pchOut );
            g_free( sz );
            return -1;
    }
    g_free( pchOut );
    g_free( sz );

    return 0;
}

static int RecordAbort( char *pchOut ) {

    remove( pchOut );
    free( pchOut );
    return 0;
}

static int RecordAddGame( list *plGame, playerrecord apr[ 2 ] ) {

    moverecord *pmr = (moverecord *) plGame->plNext->p;
    xmovegameinfo* pmgi = &pmr->g;
    int i;
    expaverage ea;
    float aaaar[ 3 ][ 2 ][ 2 ][ 2 ];
    
    assert( pmr->mt == MOVE_GAMEINFO );
    
    if( !pmgi->sc.fMoves || !pmgi->sc.fCube || !pmgi->sc.fDice )
	/* game is not completely analysed */
	return -1;

    /* ensure statistics are updated */

    updateStatisticsGame ( plGame );
    
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
	      ( pmgi->sc.arLuck[ i ][ 0 ] -
		pmgi->sc.arLuck[ 1 - i ][ 0 ] ) ) / apr[ i ].cGames;

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
		    ( pmgi->sc.arLuck[ i ][ 0 ] -
		      pmgi->sc.arLuck[ 1 - i ][ 0 ] ) *
		    ( 1.0 - arDecay[ ea - 1 ] );
	    }
    }
    
    return 0;
}

static void InitPlayerRecords( playerrecord *apr ) {

    int i;
    expaverage ea;
    
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
}

extern void CommandRecordAddGame( char *sz ) {

    FILE *pf;
    char *pch;
    playerrecord apr[ 2 ];
    
    if( !plGame ) {
	outputl( _("No game is being played.") );
	return;
    }

    InitPlayerRecords( apr );

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
    
    InitPlayerRecords( apr );
    
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

extern void CommandRecordErase( char *szPlayer ) {

    playerrecord apr[ 2 ];
    char *pch, *pchFile;
    FILE *pf;
    
    if( !( pch = NextToken( &szPlayer ) ) ) {
	outputl( _("You must specify which player's record to erase.") );
	return;
    }

    strcpy( apr[ 0 ].szName, pch );
    apr[ 0 ].cGames = -1;
    apr[ 1 ].szName[ 0 ] = 0;
    
    if( RecordRead( &pf, &pchFile, apr ) < 0 )
	return;

    if( apr[ 0 ].cGames < 0 ) {
	outputf( _("No record exists for player %s.\n"), pch );
	RecordAbort( pchFile );
	return;
    }
    
    apr[ 0 ].szName[ 0 ] = 0;
    apr[ 1 ].szName[ 0 ] = 0;
    
    if( RecordWrite( pf, pchFile, apr ) )
	return;

    outputf( _("Record for player %s erased.\n"), pch );
}

extern void CommandRecordEraseAll( char *szIgnore ) {

    char *sz = g_build_filename(szHomeDirectory, GNUBGPR, NULL);
    
    if( fConfirmSave && !GetInputYN( _("Are you sure you want to erase all "
				       "player records?") ) )
	return;

    if( g_unlink( sz ) && errno != ENOENT ) {
	/* do not complain if file is not found */
	outputerr( sz );
        g_free(sz);
	return;
    }
    g_free(sz);

    outputl( _("All player records erased.") );
}

extern void CommandRecordShow( char *szPlayer ) {

    FILE *pfIn;
    int f = FALSE;
    playerrecord pr;
    char *sz = g_build_filename(szHomeDirectory, GNUBGPR, NULL);
    
    if( !( pfIn = g_fopen( sz, "r" ) ) ) {
	if( errno == ENOENT )
	    outputl( _("No player records found.") );
	else
	    outputerr( sz );
        g_free(sz);
	return;
    }

#if USE_GTK
    if( fX )
	{
		GTKRecordShow( pfIn, sz, szPlayer );
		return;
	}
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
    g_free(sz);
}

extern playerrecord *
GetPlayerRecord( char *szPlayer ) {

    FILE *pfIn;
    static playerrecord pr;
    char *sz = g_build_filename(szHomeDirectory, GNUBGPR, NULL);
    
    if( !( pfIn = g_fopen( sz, "r" ) ) ) 
      return NULL;

    while( !RecordReadItem( pfIn, sz, &pr ) )
      if( !CompareNames( szPlayer, pr.szName ) ) {
        fclose ( pfIn );
        nVersion = 0;
        return &pr;
      }
    
    fclose( pfIn );
    g_free(sz);

    return NULL;
}
