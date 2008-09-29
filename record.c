/*
 * record.c
 *
 * by Gary Wong <gtw@gnu.org>, 2002.
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
 * $Id: record.c,v 1.34 2008/09/29 10:00:51 c_anthon Exp $
 */

#include "config.h"
#include "backgammon.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <glib/gstdio.h>

#if USE_GTK
#include "gtkgame.h"
#endif
#include "record.h"
#include "util.h"

static int anAvg[ NUM_AVG - 1 ] = { 20, 100, 500 };

/* exponential filter function, using the first three terms of the
   Taylor expansion of e^x */
#define DECAY(n) ( (1.0f - 1.0f / (n)) + 0.5f / ((n) * (n)) )
static float arDecay[ NUM_AVG - 1 ] = {
    DECAY(20), DECAY(100), DECAY(500)
};

static int nVersion; /* file format when reading player record file */
/* 0: unknown format */
/* 1: original format (locale dependent) */
/* 2: locale independent format, with version header */

extern int RecordReadItem( FILE *pf, char *pch, playerrecord *ppr ) {

    int ch, i;
    int ea;
    char ach[ 80 ];

	do
	{
		do
		{
			ch = getc( pf );
		} while(isspace(ch));

		if( ch == EOF )
		{
			nVersion = 0;

			if( feof( pf ) )
				return 1;
			else
			{
				outputerr( pch );
				return -1;
			}
		}

		if( !nVersion )
		{
			/* we don't know which format we're parsing yet */
			if (fgets( ach, 80, pf ) == NULL)
			{
				nVersion = 0;
				outputerrf( _("%s: invalid record file version"), pch );
				return -1;
			}

			if( ch != '#' || strncmp( ach, " %Version:", 10 ) ) {
				/* no version header; must be version 1 */
				nVersion = 1;
				ch = EOF;	/* reread */
				rewind( pf );
			}
			else if( ( nVersion = atoi( ach + 10 ) ) < 2 )
			{
				nVersion = 0;
				outputerrf( _("%s: invalid record file version"), pch );
				return -1;
			}
		}
	} while (ch == EOF);

    /* read and unescape name */
    i = 0;
    do
	{
		if( ch == EOF )
		{
			if( feof( pf ) )
				outputerrf( _("%s: invalid record file"), pch );
			else
				outputerr( pch );

			nVersion = 0;
			
			return -1;
		}
		else if( ch == '\\' )
		{
			unsigned int part1 = getc( pf ) & 007;
			unsigned int part2 = getc( pf ) & 007;
			unsigned int part3 = getc( pf ) & 007;
			ppr->szName[ i ] = (char)(part1 << 6 | part2 << 3 | part3);
		}
		else
			ppr->szName[ i++ ] = (char)ch;
	} while( i < 31 && !isspace( ch = getc( pf ) ) );
    ppr->szName[ i ] = 0;

    fscanf( pf, " %d ", &ppr->cGames );
    if( ppr->cGames < 0 )
		ppr->cGames = 0;
    
    for( ea = 0; ea < NUM_AVG; ea++ )
	{
			/* Not really cute, but let me guess that this will
				* be cleaned out anytime in the near future. */
		gchar str1[G_ASCII_DTOSTR_BUF_SIZE];
		gchar str2[G_ASCII_DTOSTR_BUF_SIZE];
		gchar str3[G_ASCII_DTOSTR_BUF_SIZE];
		gchar str4[G_ASCII_DTOSTR_BUF_SIZE];

		if( fscanf( pf, "%s %s %s %s ", str1, str2, str3, str4) < 4)
		{
			if( ferror( pf ) )
				outputerr( pch );
			else
				outputerrf( _("%s: invalid record file"), pch );

			nVersion = 0;

			return -1;
		}
		ppr->arErrorChequerplay[ ea ] = g_ascii_strtod(str1, NULL);
		ppr->arErrorCube[ ea ] = g_ascii_strtod(str2, NULL);
		ppr->arErrorCombined[ ea ] = g_ascii_strtod(str3, NULL);
		ppr->arLuck[ ea ] = g_ascii_strtod(str4, NULL);
	}

    return 0;
}

static int RecordWriteItem( FILE *pf, const char *pch, playerrecord *ppr ) {

    char *pchName;
    gchar buf0[G_ASCII_DTOSTR_BUF_SIZE];
    gchar buf1[G_ASCII_DTOSTR_BUF_SIZE];
    gchar buf2[G_ASCII_DTOSTR_BUF_SIZE];
    gchar buf3[G_ASCII_DTOSTR_BUF_SIZE];
    int ea;

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

    if( ferror( pf ) )
	{
		outputerr( pch );
		return -1;
    }
    
    return 0;
}

/* Reads the records and stores the ones for the current players in apr and
 * the remaining ones in the tmpfile named *ppchOut. *ppchOut will be freed
 * and *ppfOut will be closed by RecordWrite or RecordAbort */
static int RecordRead(FILE ** ppfOut, char **ppchOut, playerrecord apr[2])
{
	int n;
	playerrecord pr;
	FILE *pfIn;
	char *sz;

	*ppfOut = GetTemporaryFile("gnubgprXXXXXX", ppchOut);
	if (ppfOut == NULL)
		return -1;

	if (fputs("# %Version: 2 ($Revision: 1.34 $)\n", *ppfOut) < 0) {
		outputerr(*ppchOut);
		g_free(*ppchOut);
		return -1;
	}

	sz = g_build_filename(szHomeDirectory, GNUBGPR, NULL);
	if ((pfIn = g_fopen(sz, "r")) == NULL) {
		g_free(sz);
		/* could not read existing records; assume empty */
		return 0;
	}

	while ((n = RecordReadItem(pfIn, sz, &pr)) == 0) {
		if (!CompareNames(pr.szName, apr[0].szName))
			apr[0] = pr;
		else if (!CompareNames(pr.szName, apr[1].szName))
			apr[1] = pr;
		else if (RecordWriteItem(*ppfOut, *ppchOut, &pr) < 0) {
			n = -1;
			break;
		}
	}
	g_free(sz);
	fclose(pfIn);

	return n < 0 ? -1 : 0;
}

static int RecordWrite( FILE *pfOut, char *pchOut, playerrecord apr[ 2 ] ) {

    char *sz = g_build_filename(szHomeDirectory, GNUBGPR, NULL);

    if( RecordWriteItem( pfOut, pchOut, &apr[ 0 ] ) || RecordWriteItem( pfOut, pchOut, &apr[ 1 ] ) )
	{
		g_free( pchOut );
		return -1;
    }
    
    if( fclose( pfOut ) )
	{
		outputerr( pchOut );
		g_free( pchOut );
        g_free( sz );
		return -1;
    }
    
    if( g_rename( pchOut, sz ) )
	{
            int c;                    
            FILE *IPFile, *OPFile;

            if ( g_unlink ( sz ) && errno != ENOENT ) {
                    /* do not complain if file is not found */
                    outputerr ( sz );
                    g_free ( pchOut );
                    g_free( sz );
                    return -1;
            }

            IPFile = g_fopen(pchOut,"r");
            OPFile = g_fopen(sz,"w");
			if (!IPFile || !OPFile)
				return -1;
            while ((c = fgetc(IPFile)) != EOF)
                    fputc(c, OPFile);

            fclose(IPFile);
            fclose(OPFile);
            g_unlink(pchOut);
            free( pchOut );
            g_free( sz );
            return -1;
    }
    g_free( pchOut );
    g_free( sz );

    return 0;
}

static void RecordAbort( char *pchOut )
{
    g_unlink( pchOut );
    g_free( pchOut );
}

static int RecordAddGame( const listOLD *plNewGame, playerrecord apr[ 2 ] )
{
    moverecord *pmr = (moverecord *) plNewGame->plNext->p;
    xmovegameinfo* pmgi = &pmr->g;
    int i;
    int ea;
    float aaaar[ 3 ][ 2 ][ 2 ][ 2 ];
    
    g_assert( pmr->mt == MOVE_GAMEINFO );
    
    if( !pmgi->sc.fMoves || !pmgi->sc.fCube || !pmgi->sc.fDice )
		/* game is not completely analysed */
		return -1;

    /* ensure statistics are updated */

    updateStatisticsGame ( plNewGame );
    
    for( i = 0; i < 2; i++ )
	{
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

		for( ea = (int)EXPAVG_20; ea < NUM_AVG; ea++ )
		{
			if( apr[ i ].cGames == anAvg[ ea - 1 ] )
			{
				apr[ i ].arErrorChequerplay[ ea ] =
					apr[ i ].arErrorChequerplay[ EXPAVG_TOTAL ];
				apr[ i ].arErrorCube[ ea ] =
					apr[ i ].arErrorCube[ EXPAVG_TOTAL ];
				apr[ i ].arErrorCombined[ ea ] =
					apr[ i ].arErrorCombined[ EXPAVG_TOTAL ];
				apr[ i ].arLuck[ ea ] =
					apr[ i ].arLuck[ EXPAVG_TOTAL ];
			}
			else if( apr[ i ].cGames > anAvg[ ea - 1 ] )
			{
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
	}
    
    return 0;
}

static void InitPlayerRecords( playerrecord *apr )
{
    int i;
    int ea;
    
    for( i = 0; i < 2; i++ )
	{
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

extern void CommandRecordAddGame( char *notused )
{
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
    
    if( RecordAddGame( plGame, apr ) < 0 )
	{
		outputl( _("No game statistics are available.") );
		RecordAbort( pch );
		return;
    }
	else if( !RecordWrite( pf, pch, apr ) )
		outputl( _("Game statistics saved to player records.") );
}

extern void CommandRecordAddMatch( char *notused )
{
    listOLD *pl;
    int c = 0;
    FILE *pf;
    char *pch;
    playerrecord apr[ 2 ];
   
    if( ListEmpty( &lMatch ) )
	{
		outputl( _("No match is being played.") );
		return;
    }
    
    InitPlayerRecords( apr );
    
    if( RecordRead( &pf, &pch, apr ) < 0 )
		return;

    for( pl = lMatch.plNext; pl != &lMatch; pl = pl->plNext )
	if( RecordAddGame( pl->p, apr ) >= 0 )
	    c++;

    if( !c )
	{
		outputl( _("No game statistics are available.") );
		RecordAbort( pch );
		return;
    }
    
    if( RecordWrite( pf, pch, apr ) )
		return;

    outputf(ngettext("Statistics from %d game were saved to player records.",
			    "Statistics from %d games were saved to player records.", c), c);
}

extern void CommandRecordAddSession( char *sz )
{
    if( ListEmpty( &lMatch ) )
	{
		outputl( _("No session is being played.") );
		return;
    }

    CommandRecordAddMatch( sz );
}

extern void CommandRecordErase( char *szPlayer )
{
    playerrecord apr[ 2 ];
    char *pch, *pchFile;
    FILE *pf;
    
    if( ( pch = NextToken( &szPlayer ) ) == 0 )
	{
		outputl( _("You must specify which player's record to erase.") );
		return;
    }

    strcpy( apr[ 0 ].szName, pch );
    apr[ 0 ].cGames = -1;
    apr[ 1 ].szName[ 0 ] = 0;
    
    if( RecordRead( &pf, &pchFile, apr ) < 0 )
		return;

    if( apr[ 0 ].cGames < 0 )
	{
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

extern void CommandRecordEraseAll( char *notused )
{
    char *sz = g_build_filename(szHomeDirectory, GNUBGPR, NULL);
    
    if( fConfirmSave && !GetInputYN( _("Are you sure you want to erase all "
				       "player records?") ) )
		return;

    if( g_unlink( sz ) && errno != ENOENT )
	{
		/* do not complain if file is not found */
		outputerr( sz );
        g_free(sz);
		return;
    }
    g_free(sz);

    outputl( _("All player records erased.") );
}

extern void CommandRecordShow( char *szPlayer )
{
    FILE *pfIn;
    int f = FALSE;
    playerrecord pr;
    char *sz = g_build_filename(szHomeDirectory, GNUBGPR, NULL);
    
    if( ( pfIn = g_fopen( sz, "r" ) ) == 0 )
	{
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
	{
		if( !szPlayer || !*szPlayer || !CompareNames( szPlayer, pr.szName ) )
		{
			if( !f )
			{
				outputf("%-31s %-11s %-11s %-12s %-6s %10s\n", "", _("Short-term"), _("Long-term"), _("Total"), _("Total"), "");
				outputf("%-31s %-11s %-11s %-12s %-6s %10s\n", "", _("error rate"), _("error rate"), _("error rate"), _("luck"), "");
				outputf("%-31s %-11s %-11s %-12s %-6s %10s\n", "Name", _("Cheq. Cube"),_("Cheq. Cube"),_("Cheq. Cube"), _("rate"), _("Games"));
				f = TRUE;
			}
			outputf( "%-31s %5.3f %5.3f %5.3f %5.3f %5.3f %5.3f %6.3f %11d\n",
				pr.szName, pr.arErrorChequerplay[ EXPAVG_20 ],
				pr.arErrorCube[ EXPAVG_20 ],
				pr.arErrorChequerplay[ EXPAVG_100 ],
				pr.arErrorCube[ EXPAVG_100 ],
				pr.arErrorChequerplay[ EXPAVG_TOTAL ],
				pr.arErrorCube[ EXPAVG_TOTAL ],
				pr.arLuck[ EXPAVG_TOTAL ], pr.cGames );
		}
	}

    if( ferror( pfIn ) )
		outputerr( sz );
    else if( !f )
	{
		if( szPlayer && *szPlayer )
			outputf( _("No record for player `%s' found.\n"), szPlayer );
		else
			outputl( _("No player records found.") );
    }
    
    fclose( pfIn );
    g_free(sz);
}

extern playerrecord *
GetPlayerRecord( char *szPlayer )
{
    FILE *pfIn;
    static playerrecord pr;
    char *sz = g_build_filename(szHomeDirectory, GNUBGPR, NULL);
    
    if( ( pfIn = g_fopen( sz, "r" ) ) == NULL ) 
      return NULL;

    while( !RecordReadItem( pfIn, sz, &pr ) )
	{
      if( !CompareNames( szPlayer, pr.szName ) )
	  {
        fclose ( pfIn );
        nVersion = 0;
        return &pr;
      }
	}
    
    fclose( pfIn );
    g_free(sz);

    return NULL;
}
