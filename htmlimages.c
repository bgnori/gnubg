/*
 * htmlimages.c
 *
 * by Gary Wong <gtw@gnu.org>, 1997-2002.
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
 * $Id: htmlimages.c,v 1.17 2003/12/03 11:42:13 Superfly_Jon Exp $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <string.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_LIBART
#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_point.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_gray_svp.h>
#include <libart_lgpl/art_rgb.h>
#include <libart_lgpl/art_rgb_svp.h>
#endif

#include "backgammon.h"
#include "export.h"
#include "i18n.h"
#include "render.h"
#include "renderprefs.h"
#include "boardpos.h"
#include "boarddim.h"

#if HAVE_LIBPNG

char *szFile, *pchFile;

/* Basic size + small size value */
#define s 4
#define ss (s - 1)

/* Helpers for 2d position in arrays */
#define boardStride (BOARD_WIDTH * s * 3)
#define coord(x, y) ((x) * s * 3 + (y) * s * boardStride)
#define coordStride(x, y, stride) ((x) * s * 3 + (y) * s * stride)

unsigned char auchBoard[BOARD_WIDTH * s * 3 * BOARD_HEIGHT * s],
auchChequer[2][CHEQUER_WIDTH * s * 4 * CHEQUER_HEIGHT * s],
auchChequerLabels[12 * CHEQUER_LABEL_WIDTH * s * 3 * CHEQUER_LABEL_HEIGHT * s],
auchLo[BOARD_WIDTH * s * 4 * BORDER_HEIGHT * s],
auchHi[BOARD_WIDTH * s * 4 * BORDER_HEIGHT * s],
auchCube[CUBE_WIDTH * ss * 4 * CUBE_HEIGHT * ss],
auchCubeFaces[12 * CUBE_LABEL_WIDTH * ss * 3 * CUBE_LABEL_HEIGHT * ss],
auchDice[2][DIE_WIDTH * ss * 4 * DIE_HEIGHT * ss],
auchPips[2][ss * ss * 3];

unsigned short asRefract[2][CHEQUER_WIDTH * s * CHEQUER_HEIGHT * s];

#if HAVE_LIBART
unsigned char *auchArrow[ 2 ];
unsigned char auchMidlb[ BOARD_WIDTH * s * 3 * BOARD_HEIGHT * s ];
#endif

static void WriteImageStride(unsigned char* img, int stride, int cx, int cy)
{
	if (!WritePNG(szFile, img, stride, cx, cy))
		ProgressValueAdd(1);
}

static void WriteImage(unsigned char* img, int cx, int cy)
{
	if (!WritePNG(szFile, img, boardStride, cx, cy))
		ProgressValueAdd(1);
}

static void WriteStride(const char* name, unsigned char* img, int stride, int cx, int cy)
{
	strcpy(pchFile, name);
	WriteImageStride(img, stride, cx, cy);
}

static void Write(const char* name, unsigned char* img, int cx, int cy)
{
	strcpy(pchFile, name);
	WriteImage(img, cx, cy);
}

#if HAVE_LIBART
static void DrawArrow(int side, int player)
{ /* side 0 = left, 1 = right */
	int x, y;
	int offset_x = 0;

	memcpy( auchMidlb, auchBoard, BOARD_WIDTH * s * BOARD_HEIGHT * s * 3 );
	ArrowPosition(side /* rd.fClockwise */, s, &x, &y);

	AlphaBlendClip2( auchMidlb, boardStride,
				x, y,
				BOARD_WIDTH * s, BOARD_HEIGHT * s, 
				auchMidlb, boardStride,
				x, y, 
				auchArrow[player],
				s * ARROW_WIDTH * 4,
				0, 0,
				s * ARROW_WIDTH,
				s * ARROW_HEIGHT );

	sprintf( pchFile, "b-mid%cb-%c.png", side ? 'r' : 'l', player ? 'o' : 'x' );
	if (side == 1)
		offset_x = BOARD_WIDTH - BEAROFF_WIDTH;

	WriteImage(auchMidlb + coord(offset_x, BOARD_HEIGHT / 2 - BOARD_CENTER_HEIGHT / 2),
				BEAROFF_WIDTH * s, BOARD_CENTER_HEIGHT * s);
}
#endif

static void DrawPips(unsigned char *auchDest, int nStride,
				unsigned char *auchPip, int n)
{
	int ix, iy, afPip[9];

	afPip[0] = afPip[8] = (n == 2) || (n == 3) || (n == 4) || (n == 5) || (n == 6);
	afPip[1] = afPip[7] = 0;
	afPip[2] = afPip[6] = (n == 4) || (n == 5) || (n == 6);
	afPip[3] = afPip[5] = n == 6;
	afPip[4] = n & 1;

	for (iy = 0; iy < 3; iy++)
	{
		for (ix = 0; ix < 3; ix++)
		{
			if (afPip[iy * 3 + ix])
				CopyArea(auchDest + (1 + 2 * ix) * ss * 3 +
					(1 + 2 * iy) * ss * nStride, nStride,
					auchPip, ss * 3, ss, ss);
		}
	}
}

static void WriteImages()
{
	int i, j, k;
	unsigned char auchLabel[BOARD_WIDTH * s * 3 * BOARD_HEIGHT * s],
		auchPoint[2][2][2][POINT_WIDTH * s * 3 * DISPLAY_POINT_HEIGHT * s],
		auchOff[2][BEAROFF_WIDTH * s * 3 * DISPLAY_BEAROFF_HEIGHT * s],
		auchMidBoard[BOARD_CENTER_WIDTH * s * 3 * BOARD_CENTER_HEIGHT * s],
		auchBar[2][BAR_WIDTH * s * 3 * (POINT_HEIGHT - CUBE_HEIGHT) * s];

	/* top border-high numbers */
	CopyArea(auchLabel, boardStride, auchBoard, boardStride,
				BOARD_WIDTH * s, BORDER_HEIGHT * s);

	AlphaBlendClip( auchLabel, boardStride, 0, 0, 
				BOARD_WIDTH * s, BORDER_HEIGHT * s,
				auchLabel, boardStride, 0, 0,
				auchHi, BOARD_WIDTH * s * 4, 0, 0,
				BOARD_WIDTH * s, BORDER_HEIGHT * s );

	Write("b-hitop.png", auchLabel, BOARD_WIDTH * s, BORDER_HEIGHT * s);

	/* top border-low numbers */
	CopyArea( auchLabel, boardStride, auchBoard, boardStride,
				BOARD_WIDTH * s, BORDER_HEIGHT * s );

	AlphaBlendClip( auchLabel, boardStride, 0, 0, 
				BOARD_WIDTH * s, BORDER_HEIGHT * s,
				auchLabel, boardStride, 0, 0,
				auchLo, BOARD_WIDTH * s * 4, 0, 0,
				BOARD_WIDTH * s, BORDER_HEIGHT * s );

	Write("b-lotop.png", auchLabel, BOARD_WIDTH * s, BORDER_HEIGHT * s);

	/* empty points */
	Write("b-gd.png", auchBoard + coord(BEAROFF_WIDTH, BORDER_HEIGHT),
		POINT_WIDTH * s, POINT_HEIGHT * s);

	Write("b-rd.png", auchBoard + coord(BEAROFF_WIDTH + POINT_WIDTH, BORDER_HEIGHT),
		POINT_WIDTH * s, POINT_HEIGHT * s);

	Write("b-ru.png", auchBoard + coord(BEAROFF_WIDTH, BOARD_HEIGHT / 2 + BOARD_CENTER_HEIGHT / 2),
		POINT_WIDTH * s, POINT_HEIGHT * s);

	Write("b-gu.png", auchBoard + coord(BEAROFF_WIDTH + POINT_WIDTH, BOARD_HEIGHT / 2 + BOARD_CENTER_HEIGHT / 2),
		POINT_WIDTH * s, POINT_HEIGHT * s);

	/* upper left bearoff tray */
	Write("b-loff-x0.png", auchBoard + coord(0, BORDER_HEIGHT),
		BEAROFF_WIDTH * s, BEAROFF_HEIGHT * s);

	/* upper right bearoff tray */
	Write("b-roff-x0.png", auchBoard + coord(BOARD_WIDTH - BEAROFF_WIDTH, BORDER_HEIGHT),
		BEAROFF_WIDTH * s, BEAROFF_HEIGHT * s);

	/* lower right bearoff tray */
	Write("b-roff-o0.png", auchBoard + coord(BOARD_WIDTH - BEAROFF_WIDTH, BORDER_HEIGHT + POINT_HEIGHT + BOARD_CENTER_HEIGHT),
		BEAROFF_WIDTH * s, BEAROFF_HEIGHT * s);

	/* lower left bearoff tray */
	Write("b-loff-o0.png", auchBoard + coord(0, BORDER_HEIGHT + POINT_HEIGHT + BOARD_CENTER_HEIGHT),
		BEAROFF_WIDTH * s, BEAROFF_HEIGHT * s);

	/* bar top */
	Write("b-ct.png", auchBoard + coord(BOARD_WIDTH / 2 - BAR_WIDTH / 2, BORDER_HEIGHT),
		BAR_WIDTH * s, CUBE_HEIGHT * s);

	/* bearoff tray dividers */
#if HAVE_LIBART
	/* 4 arrows, left+right side for each player */
	DrawArrow(0, 0);
	DrawArrow(0, 1);
	DrawArrow(1, 0);
	DrawArrow(1, 1);
#else
	for (i = 0; i < 2; i++)
	{
		int offset_x = 0;
		sprintf( pchFile, "b-midlb-%c.png", i ? 'x' : 'o' );
		if (i == 1)
			offset_x = BOARD_WIDTH - BEAROFF_WIDTH;
		WriteImage(auchBoard + coord(offset_x, BOARD_HEIGHT / 2 - BOARD_CENTER_HEIGHT / 2),
			BEAROFF_WIDTH * s, BOARD_CENTER_HEIGHT * s);
	}
#endif

	/* left bearoff centre */
	strcpy( pchFile, "b-midlb.png" );
	WriteImage(auchBoard + coord(0, BOARD_HEIGHT / 2 - BOARD_CENTER_HEIGHT / 2),
		BEAROFF_WIDTH * s, BOARD_CENTER_HEIGHT * s);

	/* right bearoff centre */
	strcpy( pchFile, "b-midrb.png" );
	WriteImage(auchBoard + coord(BOARD_WIDTH - BEAROFF_WIDTH, BOARD_HEIGHT / 2 - BOARD_CENTER_HEIGHT / 2),
		BEAROFF_WIDTH * s, BOARD_CENTER_HEIGHT * s);

	/* left board centre */
	strcpy( pchFile, "b-midl.png" );
	WriteImage(auchBoard + coord(BEAROFF_WIDTH, BOARD_HEIGHT / 2 - BOARD_CENTER_HEIGHT / 2),
		BOARD_CENTER_WIDTH * s, BOARD_CENTER_HEIGHT * s);

	/* bar centre */
	strcpy( pchFile, "b-midc.png" );
	WriteImage(auchBoard + coord(BOARD_WIDTH / 2 - BAR_WIDTH / 2, BOARD_HEIGHT / 2 - BOARD_CENTER_HEIGHT / 2),
		BAR_WIDTH * s, BOARD_CENTER_HEIGHT * s);

	/* right board centre */
	strcpy( pchFile, "b-midr.png" );
	WriteImage(auchBoard + coord(BOARD_WIDTH / 2 + BAR_WIDTH / 2, BOARD_HEIGHT / 2 - BOARD_CENTER_HEIGHT / 2),
		BOARD_CENTER_WIDTH * s, BOARD_CENTER_HEIGHT * s);

	/* bar bottom */
	Write("b-cb.png", auchBoard + coord(BOARD_WIDTH / 2 - BAR_WIDTH / 2, BOARD_HEIGHT - CUBE_HEIGHT - BORDER_HEIGHT),
		BAR_WIDTH * s, CUBE_HEIGHT * s);

	/* bottom border-high numbers */
	CopyArea( auchLabel, boardStride,
				auchBoard + coord(0, BOARD_HEIGHT - BORDER_HEIGHT), boardStride,
				BOARD_WIDTH * s, BORDER_HEIGHT * s );

	AlphaBlendClip( auchLabel, boardStride, 0, 0, 
				BOARD_WIDTH * s, BORDER_HEIGHT * s,
				auchLabel, boardStride, 0, 0,
				auchHi, BOARD_WIDTH * s * 4, 0, 0,
				BOARD_WIDTH * s, BORDER_HEIGHT * s );

	Write("b-hibot.png", auchLabel, BOARD_WIDTH * s, BORDER_HEIGHT * s);

	/* bottom border-low numbers */
	CopyArea( auchLabel, boardStride,
				auchBoard + coord(0, BOARD_HEIGHT - BORDER_HEIGHT), boardStride,
				BOARD_WIDTH * s, BORDER_HEIGHT * s );

	AlphaBlendClip( auchLabel, boardStride, 0, 0, 
				BOARD_WIDTH * s, BORDER_HEIGHT * s,
				auchLabel, boardStride, 0, 0,
				auchLo, BOARD_WIDTH * s * 4, 0, 0,
				BOARD_WIDTH * s, BORDER_HEIGHT * s );

	Write("b-lobot.png", auchLabel, BOARD_WIDTH * s, BORDER_HEIGHT * s);

	/* Copy 4 points (top/bottom and both colours) */
	CopyArea(auchPoint[0][0][0], POINT_WIDTH * s * 3,
		auchBoard + coord(BEAROFF_WIDTH, BORDER_HEIGHT), boardStride,
		POINT_WIDTH * s, POINT_HEIGHT * s);
	CopyArea(auchPoint[0][0][1], POINT_WIDTH * s * 3,
		auchBoard + coord(BEAROFF_WIDTH, BORDER_HEIGHT), boardStride,
		POINT_WIDTH * s, POINT_HEIGHT * s);
	CopyArea(auchPoint[0][1][0], POINT_WIDTH * s * 3,
		auchBoard + coord(BEAROFF_WIDTH + POINT_WIDTH, BOARD_HEIGHT / 2 + BOARD_CENTER_HEIGHT / 2), boardStride,
		POINT_WIDTH * s, POINT_HEIGHT * s);
	CopyArea(auchPoint[0][1][1], POINT_WIDTH * s * 3,
		auchBoard + coord(BEAROFF_WIDTH + POINT_WIDTH, BOARD_HEIGHT / 2 + BOARD_CENTER_HEIGHT / 2), boardStride,
		POINT_WIDTH * s, POINT_HEIGHT * s);
	CopyArea(auchPoint[1][0][0], POINT_WIDTH * s * 3,
		auchBoard + coord(BEAROFF_WIDTH + POINT_WIDTH, BORDER_HEIGHT), boardStride,
		POINT_WIDTH * s, POINT_HEIGHT * s);
	CopyArea(auchPoint[1][0][1], POINT_WIDTH * s * 3,
		auchBoard + coord(BEAROFF_WIDTH + POINT_WIDTH, BORDER_HEIGHT), boardStride,
		POINT_WIDTH * s, POINT_HEIGHT * s);
	CopyArea(auchPoint[1][1][0], POINT_WIDTH * s * 3,
		auchBoard + coord(BEAROFF_WIDTH, BOARD_HEIGHT / 2 + BOARD_CENTER_HEIGHT / 2), boardStride,
		POINT_WIDTH * s, POINT_HEIGHT * s);
	CopyArea(auchPoint[1][1][1], POINT_WIDTH * s * 3,
		auchBoard + coord(BEAROFF_WIDTH, BOARD_HEIGHT / 2 + BOARD_CENTER_HEIGHT / 2), boardStride,
		POINT_WIDTH * s, POINT_HEIGHT * s);

	/* Bear off trays */
	CopyArea(auchOff[0], BEAROFF_WIDTH * s * 3,
		auchBoard + coord(BOARD_WIDTH - BEAROFF_WIDTH, BORDER_HEIGHT), boardStride,
		BEAROFF_WIDTH * s, BEAROFF_HEIGHT * s);
	CopyArea(auchOff[1], BEAROFF_WIDTH * s * 3,
		auchBoard + coord(BOARD_WIDTH - BEAROFF_WIDTH, BOARD_HEIGHT / 2 + BOARD_CENTER_HEIGHT / 2), boardStride,
		BEAROFF_WIDTH * s, BEAROFF_HEIGHT * s);

	/* Bar areas */
	CopyArea(auchBar[0], BAR_WIDTH * s * 3,
		auchBoard + coord(BOARD_WIDTH / 2 - BAR_WIDTH / 2, BORDER_HEIGHT + CUBE_HEIGHT), boardStride,
		BAR_WIDTH * s, (POINT_HEIGHT - CUBE_HEIGHT) * s);
	CopyArea(auchBar[1], BAR_WIDTH * s * 3,
		auchBoard + coord(BOARD_WIDTH / 2 - BAR_WIDTH / 2, BOARD_HEIGHT / 2 + BOARD_CENTER_HEIGHT / 2), boardStride,
		BAR_WIDTH * s, (POINT_HEIGHT - CUBE_HEIGHT) * s);

	for (i = 1; i <= 5; i++)
	{
		for (j = 0; j < 2; j++)
		{	/* 1-5 chequers, both colours, down point */
			RefractBlend(auchPoint[0][0][j] + coordStride(0, CHEQUER_HEIGHT * (i - 1), POINT_WIDTH * s * 3), POINT_WIDTH * s * 3,
				auchBoard + coord(BEAROFF_WIDTH, BORDER_HEIGHT + CHEQUER_HEIGHT * (i - 1)), boardStride,
				auchChequer[j], CHEQUER_WIDTH * s * 4,
				asRefract[j], CHEQUER_WIDTH * s,
				CHEQUER_WIDTH * s, CHEQUER_HEIGHT * s);

			sprintf(pchFile, "b-gd-%c%d.png", j ? 'o' : 'x', i);
			WriteImageStride(auchPoint[0][0][j], POINT_WIDTH * s * 3, POINT_WIDTH * s, POINT_HEIGHT * s);
		}
		for (j = 0; j < 2; j++)
		{	/* 1-5 chequers, both colours, other down point */
			RefractBlend(auchPoint[1][0][j] + coordStride(0, CHEQUER_HEIGHT * (i - 1), POINT_WIDTH * s * 3), POINT_WIDTH * s * 3,
				auchBoard + coord(BEAROFF_WIDTH + POINT_WIDTH, BORDER_HEIGHT + CHEQUER_HEIGHT * (i - 1)), boardStride,
				auchChequer[j], CHEQUER_WIDTH * s * 4,
				asRefract[j], CHEQUER_WIDTH * s,
				CHEQUER_WIDTH * s, CHEQUER_HEIGHT * s);

			sprintf(pchFile, "b-rd-%c%d.png", j ? 'o' : 'x', i);
			WriteImageStride(auchPoint[1][0][j], POINT_WIDTH * s * 3, POINT_WIDTH * s, POINT_HEIGHT * s);
		}
		for (j = 0; j < 2; j++)
		{	/* 1-5 chequers, both colours, up point */
			RefractBlend(auchPoint[0][1][j] + coordStride(0, CHEQUER_HEIGHT * (5 - i), POINT_WIDTH * s * 3), POINT_WIDTH * s * 3,
				auchBoard + coord(BEAROFF_WIDTH + POINT_WIDTH, BOARD_HEIGHT - BORDER_HEIGHT - CHEQUER_HEIGHT * i), boardStride,
				auchChequer[j], CHEQUER_WIDTH * s * 4,
				asRefract[j], CHEQUER_WIDTH * s,
				CHEQUER_WIDTH * s, CHEQUER_HEIGHT * s);

			sprintf(pchFile, "b-gu-%c%d.png", j ? 'o' : 'x', i);
			WriteImageStride(auchPoint[0][1][j], POINT_WIDTH * s * 3, POINT_WIDTH * s, POINT_HEIGHT * s);
		}
		for (j = 0; j < 2; j++)
		{	/* 1-5 chequers, both colours, other up point */
			RefractBlend(auchPoint[1][1][j] + coordStride(0, CHEQUER_HEIGHT * (5 - i), POINT_WIDTH * s * 3), POINT_WIDTH * s * 3,
				auchBoard + coord(BEAROFF_WIDTH, BOARD_HEIGHT - BORDER_HEIGHT - CHEQUER_HEIGHT * i), boardStride,
				auchChequer[j], CHEQUER_WIDTH * s * 4,
				asRefract[j], CHEQUER_WIDTH * s,
				CHEQUER_WIDTH * s, CHEQUER_HEIGHT * s);

			sprintf(pchFile, "b-ru-%c%d.png", j ? 'o' : 'x', i);
			WriteImageStride(auchPoint[1][1][j], POINT_WIDTH * s * 3, POINT_WIDTH * s, POINT_HEIGHT * s);
		}

		/* 1-5 chequers in bear-off trays */
		RefractBlend(auchOff[0] + coordStride(BORDER_WIDTH, CHEQUER_HEIGHT * (i - 1), BEAROFF_WIDTH * s * 3), BEAROFF_WIDTH * s * 3,
			auchBoard + coord(BOARD_WIDTH - BORDER_WIDTH - CHEQUER_WIDTH, BORDER_HEIGHT + CHEQUER_HEIGHT * (i - 1)), boardStride,
			auchChequer[0], CHEQUER_WIDTH * s * 4,
			asRefract[0], CHEQUER_WIDTH * s,
			CHEQUER_WIDTH * s, CHEQUER_HEIGHT * s);

		sprintf(pchFile, "b-roff-x%d.png", i);
		WriteImageStride(auchOff[0], BEAROFF_WIDTH * s * 3, BEAROFF_WIDTH * s, BEAROFF_HEIGHT * s);

		RefractBlend(auchOff[1] + coordStride(BORDER_WIDTH, CHEQUER_HEIGHT * (5 - i), BEAROFF_WIDTH * s * 3), BEAROFF_WIDTH * s * 3,
			auchBoard + coord(BOARD_WIDTH - BORDER_WIDTH - CHEQUER_WIDTH, BOARD_HEIGHT - BORDER_HEIGHT - CHEQUER_HEIGHT * i), boardStride,
			auchChequer[1], CHEQUER_WIDTH * s * 4,
			asRefract[1], CHEQUER_WIDTH * s,
			CHEQUER_WIDTH * s, CHEQUER_HEIGHT * s);

		sprintf(pchFile, "b-roff-o%d.png", i);
		WriteImageStride(auchOff[1], BEAROFF_WIDTH * s * 3, BEAROFF_WIDTH * s, BEAROFF_HEIGHT * s);
	}

	for (i = 6; i <= 15; i++)
	{
		for (j = 0; j < 2; j++)
		{	/* 6-15 chequers, both colours, down point */
			CopyArea(auchPoint[0][0][j] + coordStride(1, CHEQUER_HEIGHT * 4 + 1, POINT_WIDTH * s * 3), POINT_WIDTH * s * 3,
				auchChequerLabels + coordStride(0, (i - 4) * CHEQUER_LABEL_HEIGHT, CHEQUER_LABEL_WIDTH * s * 3), CHEQUER_LABEL_WIDTH * s * 3,
				CHEQUER_LABEL_WIDTH * s, CHEQUER_LABEL_HEIGHT * s);
			sprintf(pchFile, "b-gd-%c%d.png", j ? 'o' : 'x', i);
			WriteImageStride(auchPoint[0][0][j], POINT_WIDTH * s * 3, POINT_WIDTH * s, POINT_HEIGHT * s);
		}
		for (j = 0; j < 2; j++)
		{	/* 6-15 chequers, both colours, other down point */
			CopyArea(auchPoint[1][0][j] + coordStride(1, CHEQUER_HEIGHT * 4 + 1, POINT_WIDTH * s * 3), POINT_WIDTH * s * 3,
				auchChequerLabels + coordStride(0, (i - 4) * CHEQUER_LABEL_HEIGHT, CHEQUER_LABEL_WIDTH * s * 3), CHEQUER_LABEL_WIDTH * s * 3,
				CHEQUER_LABEL_WIDTH * s, CHEQUER_LABEL_HEIGHT * s);
			sprintf(pchFile, "b-rd-%c%d.png", j ? 'o' : 'x', i);
			WriteImageStride(auchPoint[1][0][j], POINT_WIDTH * s * 3, POINT_WIDTH * s, POINT_HEIGHT * s);
		}
		for (j = 0; j < 2; j++)
		{	/* 6-15 chequers, both colours, up point */
			CopyArea(auchPoint[0][1][j] + coordStride(1, 1, POINT_WIDTH * s * 3), POINT_WIDTH * s * 3,
				auchChequerLabels + coordStride(0, (i - 4) * CHEQUER_LABEL_HEIGHT, CHEQUER_LABEL_WIDTH * s * 3), CHEQUER_LABEL_WIDTH * s * 3,
				CHEQUER_LABEL_WIDTH * s, CHEQUER_LABEL_HEIGHT * s);
			sprintf(pchFile, "b-gu-%c%d.png", j ? 'o' : 'x', i);
			WriteImageStride(auchPoint[0][1][j], POINT_WIDTH * s * 3, POINT_WIDTH * s, POINT_HEIGHT * s);
		}
		for (j = 0; j < 2; j++)
		{	/* 6-15 chequers, both colours, other up point */
			CopyArea(auchPoint[1][1][j] + coordStride(1, 1, POINT_WIDTH * s * 3), POINT_WIDTH * s * 3,
				auchChequerLabels + coordStride(0, (i - 4) * CHEQUER_LABEL_HEIGHT, CHEQUER_LABEL_WIDTH * s * 3), CHEQUER_LABEL_WIDTH * s * 3,
				CHEQUER_LABEL_WIDTH * s, CHEQUER_LABEL_HEIGHT * s);
			sprintf(pchFile, "b-ru-%c%d.png", j ? 'o' : 'x', i);
			WriteImageStride(auchPoint[1][1][j], POINT_WIDTH * s * 3, POINT_WIDTH * s, POINT_HEIGHT * s);
		}
	
		/* 6-15 chequers in bear-off trays */
		CopyArea(auchOff[0] + coordStride(CHEQUER_LABEL_WIDTH, CHEQUER_HEIGHT * 4 + 1, BEAROFF_WIDTH * s * 3), BEAROFF_WIDTH * s * 3,
			auchChequerLabels + coordStride(0, (i - 4) * CHEQUER_LABEL_HEIGHT, CHEQUER_LABEL_WIDTH * s * 3), CHEQUER_LABEL_WIDTH * s * 3,
			CHEQUER_LABEL_WIDTH * s, CHEQUER_LABEL_HEIGHT * s);
		sprintf(pchFile, "b-roff-x%d.png", i);
		WriteImageStride(auchOff[0], BEAROFF_WIDTH * s * 3, BEAROFF_WIDTH * s, BEAROFF_HEIGHT * s);

		CopyArea(auchOff[1] + coordStride(CHEQUER_LABEL_WIDTH, 1, BEAROFF_WIDTH * s * 3), BEAROFF_WIDTH * s * 3,
			auchChequerLabels + coordStride(0, (i - 4) * CHEQUER_LABEL_HEIGHT, CHEQUER_LABEL_WIDTH * s * 3), CHEQUER_LABEL_WIDTH * s * 3,
			CHEQUER_LABEL_WIDTH * s, CHEQUER_LABEL_HEIGHT * s);
		sprintf(pchFile, "b-roff-o%d.png", i);
		WriteImageStride(auchOff[1], BEAROFF_WIDTH * s * 3, BEAROFF_WIDTH * s, BEAROFF_HEIGHT * s);
	}

	/* 0-15 chequers on bar */
	WriteStride("b-bar-o0.png", auchBar[0], BAR_WIDTH * s * 3, BAR_WIDTH * s, (POINT_HEIGHT - CUBE_HEIGHT) * s);
	WriteStride("b-bar-x0.png", auchBar[1], BAR_WIDTH * s * 3, BAR_WIDTH * s, (POINT_HEIGHT - CUBE_HEIGHT) * s);

	for (i = 1; i <= 3; i++)
	{
		RefractBlend(auchBar[0] + coordStride(BAR_WIDTH / 2 - CHEQUER_WIDTH / 2, (CHEQUER_HEIGHT + 1) * (3 - i) + 1, BAR_WIDTH * s * 3), BAR_WIDTH * s * 3,
				auchBoard + coord(BOARD_WIDTH / 2 - CHEQUER_WIDTH / 2, (BORDER_HEIGHT + CUBE_HEIGHT) + (CHEQUER_HEIGHT + 1) * (3 - i)), boardStride,
				auchChequer[0], CHEQUER_WIDTH * s * 4,
				asRefract[0], CHEQUER_WIDTH * s,
				CHEQUER_WIDTH * s, CHEQUER_HEIGHT * s);
		sprintf(pchFile, "b-bar-o%d.png", i);
		WriteImageStride(auchBar[0], BAR_WIDTH * s * 3, BAR_WIDTH * s, (POINT_HEIGHT - CUBE_HEIGHT) * s);

		RefractBlend(auchBar[1] + coordStride(BAR_WIDTH / 2 - CHEQUER_WIDTH / 2, (CHEQUER_HEIGHT + 1) * (i - 1) + 1, BAR_WIDTH * s * 3), BAR_WIDTH * s * 3,
				auchBoard + coord(BOARD_WIDTH / 2 - CHEQUER_WIDTH / 2, (BOARD_HEIGHT / 2 + BOARD_CENTER_HEIGHT / 2) + (CHEQUER_HEIGHT + 1) * (i - 1)), boardStride,
				auchChequer[1], CHEQUER_WIDTH * s * 4,
				asRefract[1], CHEQUER_WIDTH * s,
				CHEQUER_WIDTH * s, CHEQUER_HEIGHT * s);
		sprintf(pchFile, "b-bar-x%d.png", i);
		WriteImageStride(auchBar[1], BAR_WIDTH * s * 3, BAR_WIDTH * s, (POINT_HEIGHT - CUBE_HEIGHT) * s);
	}

	for (i = 4; i <= 15; i++)
	{
		CopyArea(auchBar[0] + coordStride(CHEQUER_LABEL_WIDTH, 2, BAR_WIDTH * s * 3), BAR_WIDTH * s * 3,
			auchChequerLabels + coordStride(0, (i - 4) * CHEQUER_LABEL_HEIGHT, CHEQUER_LABEL_WIDTH * s * 3), CHEQUER_LABEL_WIDTH * s * 3,
			CHEQUER_LABEL_WIDTH * s, CHEQUER_LABEL_HEIGHT * s);
		sprintf(pchFile, "b-bar-o%d.png", i);
		WriteImageStride(auchBar[0], BAR_WIDTH * s * 3, BAR_WIDTH * s, (POINT_HEIGHT - CUBE_HEIGHT) * s);

		CopyArea(auchBar[1] + coordStride(CHEQUER_LABEL_WIDTH, POINT_HEIGHT - CUBE_HEIGHT - CHEQUER_HEIGHT, BAR_WIDTH * s * 3), BAR_WIDTH * s * 3,
			auchChequerLabels + coordStride(0, (i - 4) * CHEQUER_LABEL_HEIGHT, CHEQUER_LABEL_WIDTH * s * 3), CHEQUER_LABEL_WIDTH * s * 3,
			CHEQUER_LABEL_WIDTH * s, CHEQUER_LABEL_HEIGHT * s);
		sprintf(pchFile, "b-bar-x%d.png", i);
		WriteImageStride(auchBar[1], BAR_WIDTH * s * 3, BAR_WIDTH * s, (POINT_HEIGHT - CUBE_HEIGHT) * s);
	}

	/* cube - top and bottom of bar */
	for (i = 0; i < 2; i++)
	{
		int offset;
		int cube_y = i ? ((BOARD_HEIGHT - BORDER_HEIGHT) * s - CUBE_HEIGHT * ss) : (BORDER_HEIGHT * s);

		AlphaBlend(auchBoard + (BOARD_WIDTH / 2) * s * 3 - (CUBE_WIDTH / 2) * ss * 3 +
				cube_y * boardStride, boardStride,
				auchBoard + (BOARD_WIDTH / 2) * s * 3 - (CUBE_WIDTH / 2) * ss * 3 +
				cube_y * boardStride, boardStride,
				auchCube, CUBE_WIDTH * ss * 4,
				CUBE_WIDTH * ss, CUBE_HEIGHT * ss);

		offset = i ? coord(BOARD_WIDTH / 2 - BAR_WIDTH / 2, BOARD_HEIGHT - BORDER_HEIGHT - CUBE_HEIGHT)
			: coord(BOARD_WIDTH / 2 - BAR_WIDTH / 2, BORDER_HEIGHT);

		for (j = 0; j < 12; j++)
		{
			CopyAreaRotateClip(auchBoard, boardStride, 
					(BOARD_WIDTH / 2) * s - (CUBE_WIDTH / 2) * ss + ss, cube_y + ss,
					BOARD_WIDTH * s, BOARD_HEIGHT * s,
					auchCubeFaces, CUBE_LABEL_WIDTH * ss * 3,
					0, CUBE_LABEL_HEIGHT * ss * j,
					ss * CUBE_LABEL_WIDTH, ss * CUBE_LABEL_HEIGHT,
					2 - 2 * i);

			sprintf(pchFile, "b-%s-%d.png", i ? "cb" : "ct", 2 << j);
			WriteImage(auchBoard + offset, BAR_WIDTH * s, CUBE_HEIGHT * s);

			if (j == 5)
			{	/* 64 cube is also the cube for 1 */
				sprintf(pchFile, "b-%sc-1.png", i ? "cb" : "ct");
				WriteImage(auchBoard + offset, BAR_WIDTH * s, CUBE_HEIGHT * s);
			}
		}
	}
	/* cube - doubles */
	for (i = 0; i < 2; i++)
	{
		int offset;

		CopyArea(auchMidBoard, BOARD_CENTER_WIDTH * s * 3,
				auchBoard + coord(i ? BOARD_WIDTH / 2 + BAR_WIDTH / 2 : BEAROFF_WIDTH, BOARD_HEIGHT / 2 - BOARD_CENTER_HEIGHT / 2), boardStride,
				BOARD_CENTER_WIDTH * s, BOARD_CENTER_HEIGHT * s);

		offset = ((BOARD_CENTER_HEIGHT / 2) * s - (CUBE_HEIGHT / 2) * ss) * BOARD_CENTER_WIDTH * s * 3 +
			3 * POINT_WIDTH * s * 3 - (CUBE_WIDTH / 2) * ss * 3;

		AlphaBlend(auchMidBoard + offset, BOARD_CENTER_WIDTH * s * 3,
				auchMidBoard + offset, BOARD_CENTER_WIDTH * s * 3,
				auchCube, CUBE_WIDTH * ss * 4,
				CUBE_WIDTH * ss, CUBE_HEIGHT * ss);

		for (j = 0; j < 12; j++)
		{
			CopyAreaRotateClip( auchMidBoard, BOARD_CENTER_WIDTH * s * 3,
					POINT_WIDTH * s * 3 - ss * CUBE_LABEL_WIDTH / 2,
					(BOARD_CENTER_HEIGHT / 2) * s - (CUBE_LABEL_HEIGHT / 2) * ss, 
					BOARD_CENTER_WIDTH * s, BOARD_CENTER_HEIGHT * s,
					auchCubeFaces, CUBE_LABEL_WIDTH * ss * 3,
					0, CUBE_LABEL_HEIGHT * ss * j,
					ss * CUBE_LABEL_WIDTH, ss * CUBE_LABEL_HEIGHT,
					i << 1);

			sprintf(pchFile, "b-mid%c-c%d.png", i ? 'r' : 'l', 2 << j);
			WriteImageStride(auchMidBoard, BOARD_CENTER_WIDTH * s * 3,
					BOARD_CENTER_WIDTH * s, BOARD_CENTER_HEIGHT * s);
		}
	}
	/* cube - centered */
	AlphaBlend( auchBoard + 
				((BOARD_HEIGHT / 2) * s - (CUBE_HEIGHT / 2) * ss) * boardStride +
				(BOARD_WIDTH / 2) * s * 3 - (CUBE_WIDTH / 2) * ss * 3, 
				boardStride,
				auchBoard +
				((BOARD_HEIGHT / 2) * s - (CUBE_HEIGHT / 2) * ss) * boardStride +
				(BOARD_WIDTH / 2) * s * 3 - (CUBE_WIDTH / 2) * ss * 3, 
				boardStride,
				auchCube, CUBE_WIDTH * ss *4 ,
				CUBE_WIDTH * ss, CUBE_HEIGHT * ss );

	for (j = 0; j < 12; j++)
	{
		CopyAreaRotateClip(auchBoard, boardStride, 
				(BOARD_WIDTH / 2) * s - (CUBE_WIDTH / 2) * ss + ss,
				(BOARD_HEIGHT / 2) * s - (CUBE_HEIGHT / 2) * ss + ss,
				BOARD_WIDTH * s, BOARD_HEIGHT * s,
				auchCubeFaces, CUBE_LABEL_WIDTH * ss * 3,
				0, CUBE_LABEL_HEIGHT * ss * j,
				CUBE_LABEL_WIDTH * ss, CUBE_LABEL_HEIGHT * ss,
				1);
		sprintf(pchFile, "b-midc-%d.png", 2 << j);
		WriteImage(auchBoard + coord((BOARD_WIDTH / 2) - (BAR_WIDTH / 2), ((BOARD_HEIGHT / 2) - (BOARD_CENTER_HEIGHT / 2))),
				BAR_WIDTH * s, BOARD_CENTER_HEIGHT * s);

		if (j == 5)
		{	/* 64 cube is also the cube for 1 */
			Write("b-midc-1.png", auchBoard + coord((BOARD_WIDTH / 2) - (BAR_WIDTH / 2), ((BOARD_HEIGHT / 2) - (BOARD_CENTER_HEIGHT / 2))),
					BAR_WIDTH * s, BOARD_CENTER_HEIGHT * s);
		}
	}

	/* dice rolls */
	for (i = 0; i < 2; i++)
	{
		int dice_x = (BEAROFF_WIDTH + 3 * POINT_WIDTH + 
						(6 * POINT_WIDTH + BAR_WIDTH) * i) * s * 3
						- (DIE_WIDTH / 2) * ss * 3;

		int dice_y = ((BOARD_HEIGHT / 2) * s - (DIE_HEIGHT / 2) * ss) * boardStride;

		AlphaBlend(auchBoard + dice_x - DIE_WIDTH * ss * 3 + dice_y, boardStride, 
							auchBoard + dice_x - DIE_WIDTH * ss * 3 + dice_y, boardStride,
							auchDice[i], DIE_WIDTH * ss * 4,
							DIE_WIDTH * ss, DIE_HEIGHT * ss);

		AlphaBlend(auchBoard + dice_x + DIE_WIDTH * ss * 3 + dice_y, boardStride, 
							auchBoard + dice_x + DIE_WIDTH * ss * 3 + dice_y, boardStride,
							auchDice[i], DIE_WIDTH * ss * 4,
							DIE_WIDTH * ss, DIE_HEIGHT * ss);

		for( j = 0; j < 6; j++ )
		{
			for( k = 0; k < 6; k++ )
			{
				CopyArea(auchMidBoard, BOARD_CENTER_WIDTH * s * 3, 
						auchBoard + coord(i ? BOARD_WIDTH / 2 + BAR_WIDTH / 2 : BEAROFF_WIDTH, BORDER_HEIGHT + POINT_HEIGHT), boardStride,
						BOARD_CENTER_WIDTH * s, BOARD_CENTER_HEIGHT * s);

				DrawPips(auchMidBoard + 3 * POINT_WIDTH * s * 3
						- (DIE_WIDTH / 2) * ss * 3 - DIE_WIDTH * ss * 3
						+ ((BOARD_CENTER_HEIGHT / 2) * s - (DIE_HEIGHT / 2) * ss )
						* BOARD_CENTER_WIDTH * s * 3, BOARD_CENTER_WIDTH * s * 3,
						auchPips[i], j + 1);
				DrawPips(auchMidBoard + 3 * POINT_WIDTH * s * 3
						- (DIE_WIDTH / 2) * ss * 3 + DIE_WIDTH * ss * 3
						+ ((BOARD_CENTER_HEIGHT / 2) * s - (DIE_HEIGHT / 2) * ss )
						* BOARD_CENTER_WIDTH * s * 3, BOARD_CENTER_WIDTH * s * 3,
						auchPips[i], k + 1);

				sprintf(pchFile, "b-mid%c-%c%d%d.png", i ? 'r' : 'l', i ? 'o' : 'x', j + 1, k + 1);
				WriteImageStride(auchMidBoard, BOARD_CENTER_WIDTH * s * 3,
						BOARD_CENTER_WIDTH * s, BOARD_CENTER_HEIGHT * s);
			}
		}
	}
}

static void RenderObjects()
{
	int i;
	renderdata rd;
	memcpy( &rd, &rdAppearance, sizeof( renderdata ) );

	rd.fLabels = TRUE; 
	rd.nSize = s;

	RenderBoard( &rd, auchBoard, boardStride );
	RenderChequers( &rd, auchChequer[ 0 ], auchChequer[ 1 ], asRefract[ 0 ],
			asRefract[ 1 ], CHEQUER_WIDTH * s * 4 );
	RenderChequerLabels( &rd, auchChequerLabels, CHEQUER_LABEL_WIDTH * s * 3 );

#if HAVE_LIBART
	for (i = 0; i < 2; i++)
		auchArrow[i] = art_new( art_u8, s * ARROW_WIDTH * 4 * s * ARROW_HEIGHT );

	RenderArrows( &rd, auchArrow[0], auchArrow[1], s * ARROW_WIDTH * 4 );
#endif /* HAVE_LIBART */

	RenderBoardLabels( &rd, auchLo, auchHi, BOARD_WIDTH * s * 4 );

	/* cubes and dices are rendered a bit smaller */
	rd.nSize = ss;

	RenderCube( &rd, auchCube, CUBE_WIDTH * ss * 4 );
	RenderCubeFaces( &rd, auchCubeFaces, CUBE_LABEL_WIDTH * ss * 3, auchCube, CUBE_WIDTH * ss * 4 );
	RenderDice( &rd, auchDice[ 0 ], auchDice[ 1 ], DIE_WIDTH * ss * 4 );
	RenderPips( &rd, auchPips[ 0 ], auchPips[ 1 ], ss * 3 );
}

static char* GetFilenameBase(char* sz)
{
	sz = NextToken( &sz );

	if( !sz || !*sz )
	{
		outputf( _("You must specify a file to export to (see `%s')\n" ),
			"help export htmlimages" );
		return 0;
	}

	if( mkdir( sz
#ifndef WIN32
		, 0777
#endif
		) < 0 && errno != EEXIST )
	{
		outputerr ( sz );
		return 0;
	}

	szFile = malloc( strlen( sz ) + 32 );
	strcpy( szFile, sz );

	if( szFile[ strlen( szFile ) - 1 ] != '/' )
		strcat( szFile, "/" );

	pchFile = strchr(szFile, 0);

	return szFile;
}

static void TidyObjects()
{
#if HAVE_LIBART
	int i;
	for ( i = 0; i < 2; ++i )
		art_free( auchArrow[ i ] );
#endif /* HAVE_LIBART */
	free(szFile);
}

extern void CommandExportHTMLImages(char *sz)
{
	szFile = GetFilenameBase(sz);
	if (!szFile)
		return;

	ProgressStartValue(_("Generating image:"), 342);

	RenderObjects();
	WriteImages();

	ProgressEnd();

	TidyObjects();
}

#else
extern void CommandExportHTMLImages( char * )
{
	outputl( _("This installation of GNU Backgammon was compiled without\n"
		"support for writing HTML images.") );
}
#endif
