/*
 * drawboard.h
 *
 * by Gary Wong <gary@cs.arizona.edu>, 1998-2001.
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
 * $Id: drawboard.h,v 1.11 2003/04/04 20:41:59 thyssen Exp $
 */

#ifndef _DRAWBOARD_H_
#define _DRAWBOARD_H_

extern int fClockwise; /* Player 1 moves clockwise */

extern char *DrawBoard( char *pch, int anBoard[ 2 ][ 25 ], int fRoll,
                        char *asz[], char *szMatchID, int nChequers );
/* Fill the buffer pch with a representation of the move anMove, assuming
   the board looks like anBoard.  pch must have room for 28 characters plus
   a trailing 0 (consider the move `bar/24* 23/22* 21/20* 19/18*'). */
extern char *FormatMove( char *pch, int anBoard[ 2 ][ 25 ], int anMove[ 8 ] );
extern char *FormatMovePlain( char *pch, int anBoard[ 2 ][ 25 ],
                              int anMove[ 8 ] );
extern int ParseMove( char *pch, int an[ 8 ] );
extern void CanonicalMoveOrder( int an[] );
/* Fill the buffer pch with a FIBS "boardstyle 3" description of the game. */
extern char *FIBSBoard( char *pch, int anBoard[ 2 ][ 25 ], int fRoll,
			char *szPlayer, char *szOpp, int nMatchTo,
			int nScore, int nOpponent, int nDice0, int nDice1,
			int nCube, int fCubeOwner, int fDoubled, int fTurn,
			int fCrawford, int nChequers );
/* Read a FIBS "boardstyle 3" description from pch. */
extern int ParseFIBSBoard( char *pch, int anBoard[ 2 ][ 25 ],
			   char *szPlayer, char *szOpp, int *pnMatchTo,
			   int *pnScore, int *pnScoreOpponent,
			   int anDice[ 2 ], int *pnCube, int *pfCubeOwner,
			   int *pfDoubled, int *pfTurn, int *pfCrawford );

#endif
