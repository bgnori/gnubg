/* -*- C++ -*- */

/*
 * bearoffgammon.h
 *
 * by Joseph Heled, 2000
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
 * $Id: bearoffgammon.h,v 1.8 2007/09/14 11:30:48 c_anthon Exp $
 */

#if !defined( BEAROFFGAMMON_H )
#define BEAROFFGAMMON_H

/* pack for space */
struct GammonProbs {
  unsigned int p1 : 16;  /* 0 - 36^2 */
  unsigned int p2 : 16;  /* 0 - 36^3 */
  unsigned int p3 : 24;  /* 0 - 36^4 */
  unsigned int p0 : 8;   /*  0 - 36 */
};

extern struct GammonProbs*
getBearoffGammonProbs(int b[6]);

extern long*
getRaceBGprobs(int board[6]);

#define RBG_NPROBS 5

#endif
