/*
 * dice.h
 *
 * by Gary Wong, 1999
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
 * $Id: dice.h,v 1.17 2007/03/15 22:10:57 c_anthon Exp $
 */

#ifndef _DICE_H_
#define _DICE_H_

typedef enum _rng {
    RNG_ANSI, RNG_BBS, RNG_BSD, RNG_ISAAC, RNG_MANUAL, RNG_MD5, RNG_MERSENNE, 
    RNG_RANDOM_DOT_ORG, RNG_USER, RNG_FILE,
    NUM_RNGS
} rng;

extern char *aszRNG[ NUM_RNGS ];

extern char szDiceFilename[];

extern rng rngCurrent;
extern void *rngctxCurrent;


extern void *InitRNG( int *pnSeed, int *pfInitFrom,
                      const int fSet, const rng rngx );
extern void
CloseRNG( const rng rngx, void *rngctx );
extern void DestroyRNG( const rng rngx, void **rngctx );
extern void PrintRNGSeed( const rng rngx, void *rngctx );
extern void PrintRNGCounter( const rng rngx, void *rngctx );
extern void InitRNGSeed( int n, const rng rngx, void *rngctx );
extern int
RNGSystemSeed( const rng rngx, void *p, int *pnSeed );

extern int 
RollDice( unsigned int anDice[ 2 ], const rng rngx, void *rngctx );

#if HAVE_LIBGMP
extern int InitRNGSeedLong( char *sz, rng rng, void *rngctx );
extern int InitRNGBBSModulus( char *sz, void *rngctx );
extern int InitRNGBBSFactors( char *sz0, char *sz1, void *rngctx );
#endif


#if HAVE_LIBDL
extern int
UserRNGOpen( void *p, char *sz );
#endif /* HAVE_LIBDL */

extern int
OpenDiceFile( void *rngctx, const char *sz );

extern void
CloseDiceFile( void *rngctx );

extern char *
GetDiceFileName( void *rngctx );

#endif
