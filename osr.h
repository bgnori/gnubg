/*
 * osr.h
 *
 * by J�rn Thyssen <jthyssen@dk.ibm.com>, 2002
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
 * $Id: osr.h,v 1.2 2003/05/12 18:41:01 thyssen Exp $
 */

#ifndef _OSR_H_
#define _OSR_H_

extern void
raceProbs ( int anBoard[ 2 ][ 25 ], const int nGames,
            float arOutput[ NUM_OUTPUTS ],
            float arMu[ 2 ] );


#endif /* _OSR_H_ */
