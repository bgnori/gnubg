/*
 * positionid.h
 *
 * by Gary Wong, 1998-1999
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
 * $Id: positionid.h,v 1.9 2002/03/17 13:29:01 thyssen Exp $
 */

#ifndef _POSITIONID_H_
#define _POSITIONID_H_

extern void PositionKey( int anBoard[ 2 ][ 25 ], unsigned char auchKey[ 10 ] );
extern char *PositionID( int anBoard[ 2 ][ 25 ] );
extern char *PositionIDFromKey( unsigned char auchKey[ 10 ] );
extern unsigned short PositionBearoff( int anBoard[ 6 ] );
extern void PositionFromKey( int anBoard[ 2 ][ 25 ],
                             unsigned char *puch );
extern int PositionFromID( int anBoard[ 2 ][ 25 ], char *szID );
extern void PositionFromBearoff( int anBoard[ 6 ], unsigned short usID );
extern int EqualKeys( unsigned char auch0[ 10 ], unsigned char auch1[ 10 ] );
extern int EqualBoards( int anBoard0[ 2 ][ 25 ], int anBoard1[ 2 ][ 25 ] );

extern int 
CheckPosition( int anBoard[ 2 ][ 25 ] );

extern int
LogCube ( const int n );

extern char*
MatchID ( const int nCube, const int fCubeOwner, const int fMove,
          const int nMatchTo, const int anScore[ 2 ], 
          const int fCrawford, const int anDice[ 2 ] );

extern char*
MatchIDFromKey( unsigned char auchKey[ 8 ] );

extern int
MatchFromID ( int *pnCube, int *pfCubeOwner, int *pfMove,
              int *pnMatchTo, int anScore[ 2 ], 
              int *pfCrawford, int anDice[ 2 ],
              char *szMatchID );

extern int
MatchFromKey ( int *pnCube, int *pfCubeOwner, int *pfMove,
               int *pnMatchTo, int anScore[ 2 ], 
               int *pfCrawford, int anDice[ 2 ],
               unsigned char *auchKey );

#endif
