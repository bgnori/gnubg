/*
 * mec.h
 * by Joern Thyssen, 1999-2002
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
 * $Id: mec.h,v 1.1 2004/04/16 20:06:28 thyssen Exp $
 */


#ifndef _MEC_H_
#define _MEC_H_

#include "matchequity.h"

extern void
mec( const float rGammonRate, 
     const float rWinRate,
     /* const */ float aarMetPC[ 2 ][ MAXSCORE ],
     float aarMet[ MAXSCORE ][ MAXSCORE ] );

extern void
mec_pc( const float rGammonRate, 
        const float rFreeDrop2Away, 
        const float rFreeDrop4Away, 
        const float rWinRate,
        float arMetPC[ MAXSCORE ] );

#endif /* _MEC_H_ */

