/*
 * postscript.c
 *
 * by Gary Wong <gtw@gnu.org>, 2001
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
 * $Id: postscript.c,v 1.4 2001/07/19 15:16:19 gtw Exp $
 */

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <dynarray.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "backgammon.h"
#include "drawboard.h"

typedef enum _font { FONT_NONE, FONT_RM, FONT_SF, FONT_TT } font;
static char *aszFont[ FONT_TT + 1 ] = { NULL, "rm", "sf", "tt" };

/* Output settings.  FIXME there should be commands to modify these! */
static int cxPage = 595, cyPage = 842; /* A4 -- min (451,648) */
static int nMag = 100; /* Board scale (percentage) -- min 1, max 127 */

/* Current document state. */
static int iPage, y, nFont, fPDF, idPages, idResources, idLength,
    aidFont[ FONT_TT + 1 ];
static long lStreamStart;
static font fn;
static dynarray daXRef, daPages;

static int AllocateObject( void ) {

    assert( fPDF );
    
    return DynArrayAdd( &daXRef, (void *) -1 );
}

static void StartObject( FILE *pf, int i ) {

    assert( fPDF );

    DynArraySet( &daXRef, i, (void *) ftell( pf ) );

    fprintf( pf, "%d 0 obj\n", i );
}

static void EndObject( FILE *pf ) {

    assert( fPDF );

    fputs( "endobj\n", pf );
}

static void PostScriptEscape( FILE *pf, char *pch ) {

    while( *pch ) {
	switch( *pch ) {
	case '\\':
	    fputs( "\\\\", pf );
	    break;
	case '(':
	    fputs( "\\(", pf );
	    break;
	case ')':
	    fputs( "\\)", pf );
	    break;
	default:
	    if( (unsigned char) *pch >= 0x80 )
		fprintf( pf, "\\%030o", *pch );
	    else
		putc( *pch, pf );
	    break;
	}
	pch++;
    }
}

static void StartPage( FILE *pf ) {

    iPage++;
    fn = FONT_NONE;
    y = 648;

    if( fPDF ) {
	int id, idContents;
	
	StartObject( pf, id = AllocateObject() );
	DynArrayAdd( &daPages, (void *) id );

	idContents = AllocateObject();
	fprintf( pf, "<<\n"
		 "/Type /Page\n"
		 "/Parent %d 0 R\n"
		 "/Contents %d 0 R\n"
		 "/Resources %d 0 R\n"
		 ">>\n", idPages, idContents, idResources );
	EndObject( pf );
	
	StartObject( pf, idContents );

	fprintf( pf, "<< /Length %d 0 R >>\n"
		 "stream\n", idLength = AllocateObject() );
	lStreamStart = ftell( pf );
	fprintf( pf, "1 0 0 1 %d %d cm\n", ( cxPage - 451 ) / 2,
		 ( cyPage - 648 ) / 2 );
    } else
	fprintf( pf, "%%%%Page: %d %d\n"
		 "%%%%BeginPageSetup\n"
		 "/pageobj save def %d %d translate\n"
		 "%%%%EndPageSetup\n", iPage, iPage,
		 ( cxPage - 451 ) / 2, ( cyPage - 648 ) / 2 );
}

static void EndPage( FILE *pf ) {

    if( fPDF ) {
	long cb;

	if( fn != FONT_NONE )
	    fputs( "ET\n", pf );
	
	cb = ftell( pf ) - lStreamStart;
	fputs( "endstream\n", pf );
	EndObject( pf );

	StartObject( pf, idLength );
	fprintf( pf, "%ld\n", cb );
	EndObject( pf );
    } else
	fputs( "pageobj restore showpage\n", pf );
}

static void RequestFont( FILE *pf, font fnNew, int nFontNew ) {

    if( fnNew != fn || nFontNew != nFont ) {
	if( fPDF && fn != FONT_NONE )
	    fputs( "ET\n", pf );
	
	fn = fnNew;
	nFont = nFontNew;

	if( fPDF )
	    fprintf( pf, "BT\n/%s %d Tf\n", aszFont[ fn ], nFont );
	else
	    fprintf( pf, "%d %s\n", nFont, aszFont[ fn ] );
    }
}

static void Ensure( FILE *pf, int cy ) {

    assert( cy <= 648 );
    
    if( y < cy ) {
	EndPage( pf );
	StartPage( pf );
    }
}

static void Consume( FILE *pf, int cy ) {

    y -= cy;

    assert( y >= 0 );
}

static void Advance( FILE *pf, int cy ) {

    Ensure( pf, cy );
    Consume( pf, cy );
}

static void Skip( FILE *pf, int cy ) {

    if( y < cy ) {
	EndPage( pf );
	StartPage( pf );
    } else if( y != 648 )
	Consume( pf, cy );
}

static void PostScriptPrologue( FILE *pf ) {

    /* FIXME change the font-setting procedures to use ISO 8859-1
       encoding */
    
    static char szPrologue[] =
	"%%Creator: (GNU Backgammon " VERSION ")\n"
	"%%DocumentData: Clean7Bit\n"
	"%%DocumentNeededResources: font Courier Helvetica Times-Roman\n"
	"%%DocumentSuppliedResources: procset GNU-Backgammon-Prolog 1.0 0\n"
	"%%LanguageLevel: 1\n"
	"%%Orientation: Portrait\n"
	"%%Pages: (atend)\n"
	"%%PageOrder: Ascend\n"
	"%%EndComments\n"
	"%%BeginDefaults\n"
	"%%PageResources: font Courier Helvetica Times-Roman\n"
	"%%EndDefaults\n"
	"%%BeginProlog\n"
	"%%BeginResource: procset GNU-Backgammon-Prolog 1.0 0\n"
	"\n"
	"/rm { /Times-Roman findfont exch scalefont setfont } bind def\n"
	"/sf { /Helvetica findfont exch scalefont setfont } bind def\n"
	"/tt { /Courier findfont exch scalefont setfont } bind def\n"
	"\n"
	"/cshow { dup stringwidth pop 2 div neg 0 rmoveto show } bind def\n"
	"\n"
	"/boardsection {\n"
	"  currentpoint\n"
	"  0.5 setgray\n"
	"  3 {\n"
	"    currentpoint\n"
	"    10 100 rlineto 10 -100 rlineto closepath fill\n"
	"    moveto 40 0 rmoveto\n"
	"  } repeat\n"
	"  moveto 0 setgray\n"
	"  6 { 10 100 rlineto 10 -100 rlineto } repeat\n"
	"  closepath stroke\n"
	"} bind def\n"
	"\n"	
	"/pointlabels {\n"
	"  0 1 11 {\n"
	"    3 copy 4 3 roll dup\n"
	"    5 gt { 1 add } if 20 mul 80 add\n"
	"    3 1 roll\n"
	"    1 index 13 lt { 11 exch sub } if add 2 string cvs\n"
	"    3 1 roll exch\n"
	"    moveto cshow\n"
	"  } for\n"
	"  pop pop\n"
	"} bind def\n"
	"\n"
	"/board {\n"
	"  gsave\n"
	"  currentpoint translate\n"
	"  60 10 moveto 340 10 lineto 340 250 lineto 60 250 lineto closepath "
	"stroke\n"
	"  70 20 moveto boardsection 210 20 moveto boardsection\n"
	"  180 rotate\n"
	"  -190 -240 moveto boardsection -330 -240 moveto boardsection\n"
	"  -180 rotate\n"
	"  70 20 moveto 190 20 lineto 190 240 lineto 70 240 lineto closepath "
	"stroke\n"
	"  210 20 moveto 330 20 lineto 330 240 lineto 210 240 lineto "
	"closepath stroke\n"
	"  8 sf { 242 12 } { 12 242 } ifelse 1 pointlabels 13 pointlabels\n"
	"  grestore\n"
	"} bind def\n"
	"\n"
	"/wcheq {\n"
	"  newpath 2 copy 10 0 360 arc 1 setgray fill 10 0 360 arc 0 setgray "
	"stroke\n"
	"} bind def\n"
	"\n"
	"/bcheq {\n"
	"  newpath 10 0 360 arc fill\n"
	"} bind def\n"
	"\n"
	"/label {\n"
	"  gsave\n"
	"  3 1 roll newpath moveto currentpoint\n"
	"  -5 -5 rmoveto currentpoint\n"
	"  10 0 rlineto 0 10 rlineto -10 0 rlineto closepath\n"
	"  1 setgray fill\n"
	"  newpath moveto\n"
	"  10 0 rlineto 0 10 rlineto -10 0 rlineto closepath\n"
	"  0 setgray stroke\n"
	"  8 sf moveto 0 -3 rmoveto 2 string cvs cshow\n"
	"  grestore\n"
	"} bind def\n"
	"\n"
	"/cube {\n"
	"  gsave\n"
	"  exch newpath 35 exch moveto currentpoint\n"
	"  -12 -12 rmoveto 24 0 rlineto 0 24 rlineto -24 0 rlineto closepath "
	"stroke\n"
	"  18 sf translate rotate 0 -6 moveto 4 string cvs cshow\n"
	"  grestore\n"
	"} bind def\n"
	"\n"
	"%%EndResource\n"
	"%%EndProlog\n"
	"%%BeginSetup\n"
	"%%IncludeResource: font Courier\n"
	"%%IncludeResource: font Helvetica\n"
	"%%IncludeResource: font Times-Roman\n"
	"%%EndSetup\n";
    time_t t;
    char *sz, *pch;

    if( fPDF ) {
	fputs( "%PDF-1.3\n"
	       /* binary rubbish */
	       "%\307\361\374\r\n", pf );
	
	DynArrayCreate( &daXRef, 64, TRUE );
	DynArrayCreate( &daPages, 32, TRUE );

	AllocateObject(); /* object 0 is always free */
	
	StartObject( pf, AllocateObject() ); /* object 1 -- the root */
	fprintf( pf, "<<\n"
		 "/Type /Catalog\n"
		 "/Pages %d 0 R\n"
		 ">>\n",
		 /* FIXME add outline object */
		 idPages = AllocateObject() );
	EndObject( pf );

	StartObject( pf, aidFont[ FONT_RM ] = AllocateObject() );
	fputs( "<<\n"
	       "/Type /Font\n"
	       "/Subtype /Type1\n"
	       "/Name /rm\n"
	       "/BaseFont /Times-Roman\n"
	       "/Encoding /WinAnsiEncoding\n"
	       ">>\n", pf );
	EndObject( pf );
	
	StartObject( pf, aidFont[ FONT_SF ] = AllocateObject() );
	fputs( "<<\n"
	       "/Type /Font\n"
	       "/Subtype /Type1\n"
	       "/Name /sf\n"
	       "/BaseFont /Helvetica\n"
	       "/Encoding /WinAnsiEncoding\n"
	       ">>\n", pf );
	EndObject( pf );
	
	StartObject( pf, aidFont[ FONT_TT ] = AllocateObject() );
	fputs( "<<\n"
	       "/Type /Font\n"
	       "/Subtype /Type1\n"
	       "/Name /tt\n"
	       "/BaseFont /Courier\n"
	       "/Encoding /WinAnsiEncoding\n"
	       ">>\n", pf );
	EndObject( pf );
	
	StartObject( pf, idResources = AllocateObject() );
	fprintf( pf, "<<\n"
		 "/ProcSet [/PDF /Text]\n"
		 "/Font << /rm %d 0 R /sf %d 0 R /tt %d 0 R >>\n"
		 ">>\n", aidFont[ FONT_RM ], aidFont[ FONT_SF ],
		 aidFont[ FONT_TT ] );
	/* FIXME list xobject resources */
	EndObject( pf );
    } else {
	fputs( "%!PS-Adobe-3.0\n", pf );

	fprintf( pf, "%%%%BoundingBox: %d %d %d %d\n",
		 ( cxPage - 451 ) / 2, ( cyPage - 648 ) / 2,
		 ( cxPage + 451 ) / 2, ( cyPage + 648 ) / 2 );
	
	time( &t );
	sz = ctime( &t );
	if( ( pch = strchr( sz, '\n' ) ) )
	    *pch = 0;
	fprintf( pf, "%%%%CreationDate: (%s)\n", sz );
	
	fputs( "%%Title: (", pf );
	PostScriptEscape( pf, ap[ 0 ].szName );
	fputs( " vs. ", pf );
	PostScriptEscape( pf, ap[ 1 ].szName );
	fputs( ")\n", pf );
	
	fputs( szPrologue, pf );
    }
    
    iPage = 0;
    
    StartPage( pf );
}

static void DrawPostScriptPoint( FILE *pf, int i, int fPlayer, int c ) {

    int j, x, y;

    if( i < 6 )
	x = 320 - 20 * i;
    else if( i < 12 )
	x = 300 - 20 * i;
    else if( i < 18 )
	x = 20 * i - 160;
    else if( i < 24 )
	x = 20 * i - 140;
    else if( i == 24 ) /* bar */
	x = 200;
    else /* off */
	x = 365;

    for( j = 0; j < c; j++ ) {
	if( j == 5 || ( i == 24 && j == 3 ) ) {
	    if( fPDF ) {
		fprintf( pf, "1 g %d %d 10 10 re b 0 g\n", x - 5, y - 5 );
		fprintf( pf, "BT /sf 8 Tf 1 0 0 1 %d %d Tm (%d) Tj ET\n",
			 x - ( c > 9 ? 5 : 2 ), y - 3, c );
	    } else
		fprintf( pf, "%d %d %d label\n", x, y, c );
	    
	    break;
	}
	
	if( i == 24 )
	    /* bar */
	    y = fPlayer ? 60 + 20 * j : 200 - 20 * j;
	else if( fPlayer )
	    y = i < 12 || i == 25 ? 30 + 20 * j : 230 - 20 * j;
	else
	    y = i >= 12 && i != 25 ? 30 + 20 * j : 230 - 20 * j;

	if( fPDF ) {
	    fprintf( pf, "%d g ", fPlayer ? 0 : 1 );
	    fprintf( pf, "%d %d m %d %d %d %d %d %d c\n",
		     x - 10, y, x - 10, y + 5, x - 5, y + 10, x, y + 10 );
	    fprintf( pf, "%d %d %d %d %d %d c\n",
		     x + 5, y + 10, x + 10, y + 5, x + 10, y );
	    fprintf( pf, "%d %d %d %d %d %d c\n",
		     x + 10, y - 5, x + 5, y - 10, x, y - 10);
	    fprintf( pf, "%d %d %d %d %d %d c %c\n",
		     x - 5, y - 10, x - 10, y - 5, x - 10, y,
		     fPlayer ? 'f' : 'b' );
	} else
	    fprintf( pf, "%d %d %ccheq\n", x, y, fPlayer ? 'b' : 'w' );
    }
}

static void PrintPostScriptBoard( FILE *pf, matchstate *pms, int fPlayer ) {

    int yCube, theta, anOff[ 2 ] = { 15, 15 }, i;

    if( pms->fCubeOwner < 0 ) {
	yCube = 130;
	theta = 90;
    } else if( pms->fCubeOwner ) {
	yCube = 30;
	theta = 0;
    } else {
	yCube = 230;
	theta = 180;
    }
    
    Advance( pf, 260 * nMag / 100 );

    if( fPDF ) {
	if( fn != FONT_NONE ) {
	    fputs( "ET\n", pf );
	    fn = FONT_NONE;
	}

	/* FIXME most of the following could be encapsulated into a PDF
	   XObject to optimise the output file */
	fprintf( pf, "q 1 0 0 1 %d %d cm %.2f 0 0 %.2f 0 0 cm 0.5 g\n",
		 225 - 200 * nMag / 100, y, nMag / 100.0, nMag / 100.0 );
	fputs( "60 10 280 240 re S\n", pf );
	for( i = 0; i < 6; i++ ) {
	    fprintf( pf, "%d %d m %d %d l %d %d l %c\n",
		     70 + i * 20, 20, 80 + i * 20, 120, 90 + i * 20, 20,
		     i & 1 ? 's' : 'b' );
	    fprintf( pf, "%d %d m %d %d l %d %d l %c\n",
		     210 + i * 20, 20, 220 + i * 20, 120, 230 + i * 20, 20,
		     i & 1 ? 's' : 'b' );
	    fprintf( pf, "%d %d m %d %d l %d %d l %c\n",
		     70 + i * 20, 240, 80 + i * 20, 140, 90 + i * 20, 240,
		     i & 1 ? 'b' : 's' );
	    fprintf( pf, "%d %d m %d %d l %d %d l %c\n",
		     210 + i * 20, 240, 220 + i * 20, 140, 230 + i * 20, 240,
		     i & 1 ? 'b' : 's' );
	}
	fputs( "70 20 120 220 re 210 20 120 220 re S 0 g\n", pf );
	
	for( i = 1; i <= 12; i++ )
	    fprintf( pf, "BT /sf 8 Tf 1 0 0 1 %d %d Tm (%d) Tj ET\n",
		     340 - i * 20 - ( i > 6 ) * 20 - ( i > 9 ? 5 : 2 ),
		     fPlayer ? 12 : 242, i );

	for( i = 13; i <= 24; i++ )
	    fprintf( pf, "BT /sf 8 Tf 1 0 0 1 %d %d Tm (%d) Tj ET\n",
		     i * 20 - 185 + ( i > 18 ) * 20, fPlayer ? 242 : 12, i );

	fprintf( pf, "q %d %d %d %d 35 %d cm -12 -12 24 24 re S\n"
		 "BT /sf 18 Tf 1 0 0 1 %d -6 Tm (%d) Tj ET Q\n",
		 1 - theta / 90, theta == 90, -( theta == 90 ), 1 - theta / 90,
		 yCube, pms->nCube == 1 || pms->nCube > 8 ? -10 : -5,
		 pms->nCube == 1 ? 64 : pms->nCube );
    } else
	fprintf( pf, "gsave\n"
		 "%d %d translate %.2f %.2f scale\n"
		 "0 0 moveto %s board\n"
		 "%d %d %d cube\n", 225 - 200 * nMag / 100, y,
		 nMag / 100.0, nMag / 100.0, fPlayer ? "true" : "false",
		 pms->nCube == 1 ? 64 : pms->nCube, yCube, theta );

    for( i = 0; i < 25; i++ ) {
	anOff[ 0 ] -= pms->anBoard[ 0 ][ i ];
	anOff[ 1 ] -= pms->anBoard[ 1 ][ i ];

	DrawPostScriptPoint( pf, i, 0, pms->anBoard[ !fPlayer ][ i ] );
	DrawPostScriptPoint( pf, i, 1, pms->anBoard[ fPlayer ][ i ] );
    }

    DrawPostScriptPoint( pf, 25, 0, anOff[ !fPlayer ] );
    DrawPostScriptPoint( pf, 25, 1, anOff[ fPlayer ] );    

    if( fPDF )
	fputs( "Q\n", pf );
    else
	fputs( "grestore\n", pf );
}

static short acxTimesRoman[ 256 ] = {
    /* Width of Times-Roman font in ISO 8859-1 encoding. */
    250, 250, 250, 250, 250, 250, 250, 250, /*   0 to   7 */
    250, 250, 250, 250, 250, 250, 250, 250, /*   8 to  15 */
    250, 250, 250, 250, 250, 250, 250, 250, /*  16 to  23 */
    250, 250, 250, 250, 250, 250, 250, 250, /*  24 to  31 */
    250, 333, 408, 500, 500, 833, 778, 333, /*  32 to  39 */
    333, 333, 500, 564, 250, 333, 250, 278, /*  40 to  47 */
    500, 500, 500, 500, 500, 500, 500, 500, /*  48 to  55 */
    500, 500, 278, 278, 564, 564, 564, 444, /*  56 to  63 */
    921, 722, 667, 667, 722, 611, 556, 722, /*  64 to  71 */
    722, 333, 389, 722, 611, 889, 722, 722, /*  72 to  79 */
    556, 722, 667, 556, 611, 722, 722, 944, /*  80 to  87 */
    722, 722, 611, 333, 278, 333, 469, 500, /*  88 to  95 */
    333, 444, 500, 444, 500, 444, 333, 500, /*  96 to 103 */
    500, 278, 278, 500, 278, 778, 500, 500, /* 104 to 111 */
    500, 500, 333, 389, 278, 500, 500, 722, /* 112 to 119 */
    500, 500, 444, 480, 200, 480, 541, 250, /* 120 to 127 */
    250, 250, 250, 250, 250, 250, 250, 250, /* 128 to 135 */
    250, 250, 250, 250, 250, 250, 250, 250, /* 136 to 143 */
    250, 250, 250, 250, 250, 250, 250, 250, /* 144 to 151 */
    250, 250, 250, 250, 250, 250, 250, 250, /* 152 to 159 */
    250, 333, 500, 500, 500, 500, 200, 500, /* 160 to 167 */
    333, 760, 276, 500, 564, 333, 760, 333, /* 168 to 175 */
    400, 564, 300, 300, 333, 500, 453, 250, /* 176 to 183 */
    333, 300, 310, 500, 750, 750, 750, 444, /* 184 to 191 */
    722, 722, 722, 722, 722, 722, 889, 667, /* 192 to 199 */
    611, 611, 611, 611, 333, 333, 333, 333, /* 200 to 207 */
    722, 722, 722, 722, 722, 722, 722, 564, /* 208 to 215 */
    722, 722, 722, 722, 722, 722, 556, 500, /* 216 to 223 */
    444, 444, 444, 444, 444, 444, 667, 444, /* 224 to 231 */
    444, 444, 444, 444, 278, 278, 278, 278, /* 232 to 239 */
    500, 500, 500, 500, 500, 500, 500, 564, /* 240 to 247 */
    500, 500, 500, 500, 500, 500, 500, 500  /* 248 to 255 */
};

static int StringWidth( unsigned char *sz ) {

    unsigned char *pch;
    int c;
    
    switch( fn ) {
    case FONT_RM:
	for( c = 0, pch = sz; *pch; pch++ )
	    if( isprint( *pch ) )
		c += acxTimesRoman[ *pch ];
	break;
	
    case FONT_TT:
	for( c = 0, pch = sz; *pch; pch++ )
	    if( isprint( *pch ) )
		c += 600; /* fixed Courier character width */
	break;
	
    default:
	abort();
    }

    return c * nFont / 1000;
}

/* Typeset a line of text.  We're not TeX, so we don't do any kerning,
   hyphenation or justification.  Word wrapping will have to do. */
static void PrintPostScriptComment( FILE *pf, unsigned char *pch ) {

    int x;
    unsigned char *pchStart, *pchBreak;
    
    if( !pch || !*pch )
	return;

    Skip( pf, 6 );

    while( *pch ) {
	Advance( pf, 10 );
	
	pchBreak = NULL;
	pchStart = pch;
	
	for( x = 0; x < 451 * 1000 / 10; x += acxTimesRoman[ *pch++ ] )
	    if( !*pch ) {
		/* finished; break here */
		pchBreak = pch;
		break;
	    } else if( *pch == '\n' ) {
		/* new line; break immediately */
		pchBreak = pch++;
		break;
	    } else if( *pch == ' ' )
		/* space; note candidate for breaking */
		pchBreak = pch;

	if( !pchBreak )
	    pchBreak = ( pch == pchStart ) ? pch : pch - 1;

	RequestFont( pf, FONT_RM, 10 );

	fprintf( pf, fPDF ? "1 0 0 1 0 %d Tm (" : "0 %d moveto (", y );
	
	for( ; pchStart <= pchBreak; pchStart++ )
	    switch( *pchStart ) {
	    case 0:
	    case '\n':
		break;
	    case '\\':
	    case '(':
	    case ')':
		putc( '\\', pf );
		/* fall through */
	    default:
		putc( *pchStart, pf );
	    }
	fputs( fPDF ? ") Tj\n" : ") show\n", pf );

	if( !*pch )
	    break;
	
	pch = pchBreak + 1;
    }
}

static void PrintPostScriptCubeAnalysis( FILE *pf, matchstate *pms,
					 int fPlayer, float arDouble[ 4 ],
					 evalsetup *pes ) { 
    cubeinfo ci;
    char sz[ 1024 ], *pch, *pchNext;

    if( pes->et == EVAL_NONE )
	return;
    
    SetCubeInfo( &ci, pms->nCube, pms->fCubeOwner, fPlayer, pms->nMatchTo,
		 pms->anScore, pms->fCrawford, fJacoby, fBeavers );
    
    if( !GetDPEq( NULL, NULL, &ci ) )
	/* No cube action possible */
	return;
    
    GetCubeActionSz( arDouble, sz, &ci, fOutputMWC, FALSE );

    Skip( pf, 4 );
    for( pch = sz; pch && *pch; pch = pchNext ) {
	Advance( pf, 8 );
	RequestFont( pf, FONT_TT, 8 );
	fprintf( pf, fPDF ? "1 0 0 1 144 %d Tm (" : "144 %d moveto (", y );
	if( ( pchNext = strchr( pch, '\n' ) ) )
	    *pchNext++ = 0;
	fputs( pch, pf );
	fputs( fPDF ? ") Tj\n" : ") show\n", pf );
    }
    Skip( pf, 6 );
}

static void ExportGamePostScript( FILE *pf, list *plGame ) {

    list *pl;
    moverecord *pmr;
    matchstate msExport;
    int fTook = FALSE, i;
    char sz[ 1024 ], *pch;

    for( pl = plGame->plNext; pl != plGame; pl = pl->plNext ) {
	pmr = pl->p;
	switch( pmr->mt ) {
	case MOVE_GAMEINFO:
	    /* FIXME game introduction */
	    break;
	    
	case MOVE_NORMAL:
	    if( fTook )
		/* no need to print board following a double/take */
		fTook = FALSE;
	    else {
		Ensure( pf, 260 * nMag / 100 + 10 );
		PrintPostScriptBoard( pf, &msExport, pmr->n.fPlayer );
	    }
	    
	    PrintPostScriptCubeAnalysis( pf, &msExport, pmr->n.fPlayer,
					 pmr->n.arDouble, &pmr->n.esDouble );
	    
	    Advance( pf, 10 );
	    FormatMove( sz, msExport.anBoard, pmr->n.anMove );
	    RequestFont( pf, FONT_RM, 10 );
	    fprintf( pf, fPDF ? "1 0 0 1 %d %d Tm (%d%d%s: %s%s) Tj\n" :
		     "%d %d moveto (%d%d%s: %s%s) show\n",
		     225 - ( 10 + StringWidth( aszLuckTypeAbbr[ pmr->n.lt ] ) +
			     StringWidth( aszSkillTypeAbbr[ pmr->n.st ] ) +
			     StringWidth( sz ) ) / 2, y,
		     pmr->n.anRoll[ 0 ], pmr->n.anRoll[ 1 ],
		     aszLuckTypeAbbr[ pmr->n.lt ], sz,
		     aszSkillTypeAbbr[ pmr->n.st ] );

	    if( pmr->n.ml.cMoves ) {
		Skip( pf, 4 );
		for( i = 0; i < pmr->n.ml.cMoves && i <= pmr->n.iMove; i++ ) {
		    if( i >= 5 /* FIXME allow user to choose limit */ &&
			i != pmr->n.iMove )
			continue;

		    Ensure( pf, 16 );
		    Consume( pf, 8 );
		    RequestFont( pf, FONT_TT, 8 );
		    fprintf( pf, fPDF ? "1 0 0 1 46 %d Tm (" :
			     "46 %d moveto (", y );
		    putc( i == pmr->n.iMove ? '*' : ' ', pf );
		    FormatMoveHint( sz, msExport.anBoard, &pmr->n.ml, i,
				    i != pmr->n.iMove ||
				    i != pmr->n.ml.cMoves - 1 );
		    pch = strchr( sz, '\n' );
		    *pch++ = 0;
		    fputs( sz, pf );
		    fputs( fPDF ? ") Tj\n" : ") show\n", pf );
		    
		    Consume( pf, 8 );
		    fprintf( pf, fPDF ? "1 0 0 1 46 %d Tm (" :
			     "46 %d moveto (", y );
		    putc( ' ', pf );
		    *strchr( pch, '\n' ) = 0;
		    fputs( pch, pf );
		    fputs( fPDF ? ") Tj\n" : ") show\n", pf );
		}
	    }

	    PrintPostScriptComment( pf, pmr->a.sz );

	    Skip( pf, 6 );
	    
	    break;
	    
	case MOVE_DOUBLE:	    
	    Ensure( pf, 260 * nMag / 100 );
	    PrintPostScriptBoard( pf, &msExport, pmr->d.fPlayer );

	    PrintPostScriptCubeAnalysis( pf, &msExport, pmr->d.fPlayer,
					 pmr->d.arDouble, &pmr->d.esDouble );

	    Advance( pf, 10 );
	    RequestFont( pf, FONT_RM, 10 );
	    /* (Double) stringwidth = 29.439 */
	    fprintf( pf, fPDF ? "1 0 0 1 210 %d Tm (Double) Tj\n" :
		     "210 %d moveto (Double) show\n", y );

	    PrintPostScriptComment( pf, pmr->a.sz );

	    Skip( pf, 6 );
	    
	    break;
	    
	case MOVE_TAKE:
	    fTook = TRUE;

	    Advance( pf, 10 );
	    RequestFont( pf, FONT_RM, 10 );
	    /* (Take) stringwidth = 19.9892 */
	    fprintf( pf, fPDF ? "1 0 0 1 215 %d Tm (Take) Tj\n" :
		     "215 %d moveto (Take) show\n", y );

	    PrintPostScriptComment( pf, pmr->a.sz );

	    Skip( pf, 6 );
	    
	    break;
	    
	case MOVE_DROP:
	    Advance( pf, 10 );
	    RequestFont( pf, FONT_RM, 10 );
	    /* (Drop) stringwidth = 20.5494 */
	    fprintf( pf, fPDF ? "1 0 0 1 215 %d Tm (Drop) Tj\n" :
		     "215 %d moveto (Drop) show\n", y );

	    PrintPostScriptComment( pf, pmr->a.sz );

	    Skip( pf, 6 );

	    break;
	    
	case MOVE_RESIGN:
	    /* FIXME print board? */
	    /* FIXME print resign */
	    /* FIXME print resignation analysis, if available */
	    PrintPostScriptComment( pf, pmr->a.sz );

	    Skip( pf, 6 );

	    break;
	    	    
	case MOVE_SETDICE:
	    /* ignore */
	    break;
	    
	case MOVE_SETBOARD:
	case MOVE_SETCUBEVAL:
	case MOVE_SETCUBEPOS:
	    /* FIXME print something? */
	    break;
	}

	ApplyMoveRecord( &msExport, pmr );
    }
    
    if( ( GameStatus( msExport.anBoard ) ) )
	/* FIXME print game result */
	;
    
}

static void PostScriptEpilogue( FILE *pf ) {

    int i;
    long lXRef;
    
    EndPage( pf );

    if( fPDF ) {
	StartObject( pf, idPages );
	fputs( "<<\n"
	       "/Type /Pages\n"
	       "/Kids [\n", pf );
	for( i = 0; i < daPages.c; i++ )
	    fprintf( pf, " %d 0 R\n", (int) daPages.ap[ i ] );
	fprintf( pf, "]\n"
		 "/Count %d\n"
		 "/MediaBox [0 0 %d %d]\n"
		 ">>\n", daPages.c, cxPage, cyPage );
	EndObject( pf );
	
	lXRef = ftell( pf );
	
	fprintf( pf, "xref\n"
		 "0 %d\n", daXRef.iFinish );
	
	fputs( "0000000000 65535 f \n", pf );
	
	for( i = 1; i < daXRef.iFinish; i++ ) {
	    assert( daXRef.ap[ i ] );
	    
	    fprintf( pf, "%010ld 00000 n \n", (long) daXRef.ap[ i ] );
	}

	fprintf( pf, "trailer\n"
		 "<< /Size %d /Root 1 0 R >>\n"
		 "startxref\n"
		 "%ld\n"
		 "%%%%EOF\n", daXRef.iFinish, lXRef );
    } else
	fprintf( pf, "%%%%Trailer\n"
		 "%%%%Pages: %d\n"
		 "%%%%EOF\n", iPage );
}

static void ExportGameGeneral( int f, char *sz ) {

    FILE *pf;
    
    if( !sz || !*sz ) {
	outputl( "You must specify a file to export to (see `help export"
		 "game postscript')." ); /* FIXME not necessarily PS */
	return;
    }

    if( !strcmp( sz, "-" ) ) {
	if( f ) {
	    outputl( "PDF files may not be written to standard output ("
		     "see `help export game pdf')." );
	    return;
	}
	
	pf = stdout;
    } else if( !( pf = fopen( sz, f ? "wb" : "w" ) ) ) {
	perror( sz );
	return;
    }

    fPDF = f;
    PostScriptPrologue( pf );
    
    ExportGamePostScript( pf, plGame );

    PostScriptEpilogue( pf );
    
    if( pf != stdout )
	fclose( pf );
}

extern void CommandExportGamePDF( char *sz ) {

    ExportGameGeneral( TRUE, sz );
}

extern void CommandExportGamePostScript( char *sz ) {

    ExportGameGeneral( FALSE, sz );
}

static void ExportMatchGeneral( int f, char *sz ) {
    
    FILE *pf;
    list *pl;

    if( !sz || !*sz ) {
	outputl( "You must specify a file to export to (see `help export "
		 "match postscript')." ); /* FIXME not necessarily PS */
	return;
    }

    if( !strcmp( sz, "-" ) ) {
	if( f ) {
	    outputl( "PDF files may not be written to standard output ("
		     "see `help export match pdf')." );
	    return;
	}
	
	pf = stdout;
    } else if( !( pf = fopen( sz, f ? "wb" : "w" ) ) ) {
	perror( sz );
	return;
    }

    fPDF = f;
    PostScriptPrologue( pf );

    /* FIXME write match introduction? */
    
    for( pl = lMatch.plNext; pl != &lMatch; pl = pl->plNext )
	ExportGamePostScript( pf, pl->p );
    
    PostScriptEpilogue( pf );
    
    if( pf != stdout )
	fclose( pf );
}

extern void CommandExportMatchPDF( char *sz ) {

    ExportMatchGeneral( TRUE, sz );
}

extern void CommandExportMatchPostScript( char *sz ) {

    ExportMatchGeneral( FALSE, sz );
}
