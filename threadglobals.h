/*
 * threadglobals.h
 *
 * by Olivier Baur <olivier.baur@noos.fr>, 2003
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
 */

#ifndef _THREADGLOBALS_H_
#define _THREADGLOBALS_H_ 1

/* This file must be included by all .c files using
    globals (and static locals) to declare them and
    make then thread-dependent.
    
    Use DECLARE_THREADGLOBAL(t,v) and DECLARE_THREADSTATICLOCAL(t,v) 
    to declare the thread-dependent variables, and
    THREADGLOBAL(v) to access them.
*/

#if PROCESSING_UNITS
    /* threaded case: no declaration, storage for global vars 
        is actually in threadglobals structure... */
    #define DECLARE_THREADGLOBAL(t,v,i) 
    #define DECLARE_THREADSTATICGLOBAL(t,v,i) 
    #define DECLARE_THREADSTATICLOCAL(t,v,i) 
    /* the threadglobals structure is found using the "global storage"
        key created by InitThreadGlobalStorage() -- see "procunits.h" */
    #define THREADGLOBAL(v) (((threadglobals *) pthread_getspecific(tlsThreadGlobalsKey))->v)
#else
    /* non-threaded case: usual declarations... */
    #define DECLARE_THREADGLOBAL(t,v,i) t v = i
    #define DECLARE_THREADSTATICGLOBAL(t,v,i) t v = i
    #define DECLARE_THREADSTATICLOCAL(t,v,i) static t v = i
    /* ...and usual variable usage */
    #define THREADGLOBAL(v) v
#endif


#if PROCESSING_UNITS

#include "cache.h"
#include "eval.h"
#include "lib/neuralnet.h"

/* list here all globals that are thread-dependent */
/* initialisers are in CreateThreadGlobalStorage() 
    in "procunits.c" */
typedef struct {
    /* from "eval.c" */
    float	rCubeX;				/* global */
    int		nContext[3];			/* global */
    int		nReductionGroup;		/* global */
    int		nNextContext;			/* local to ScoreMoves() */
    move 	amMoves[MAX_INCOMPLETE_MOVES];	/* local to GenerateMoves() */
    //cache	cEval;				/* global */
    //int	cCache;				/* global */
    cacheNode	result;				/* local to CacheLookup() */
    /* from "neuralnet.c" */
    void	*gSavedBasesList;		/* global */
    /* from "rollout.c" */
    int		nSkip; 				/* local to RolloutDice */
    /* from "procunits.c" */
    char 	*gpPackedData;			/* global */
    int		gPackedDataPos;			/* global */
    int		gPackedDataSize;		/* global */
} threadglobals;

/* default values are zero; add all needed initialisers in macro below;
    this macro is used by CreateThreadGlobalStorage() in "procunits.c" */
#define GLOBALS_INITIALISERS(ptg) \
    ptg->rCubeX = 2.0 / 3.0; \
    ptg->nContext[0] = -1; \
    ptg->nContext[1] = -1; \
    ptg->nContext[2] = -1; 

extern pthread_key_t tlsThreadGlobalsKey;


/* --------------------------------------------------
    THREAD GLOBALS FUNCTIONS

    These functions are used to make globals thread-
    dependent (common globals AND static locals).
    All globals that should be thread-dependent must
    be declared in the threadglobals structure.
    All globals access (r-value and l-values) must
    be done through the ThreadGlobal() macro.
    Each time a new thread is started,
    CreateThreadGlobalStorage() must be called from
    that thread before it accesses any thread-
    dependent globals.
    In the case where several static locals have been
    defined with the same name in different functions,
    those static locals should be renamed using unique
    names in the program scope.
   --------------------------------------------------
*/


extern int gDebugDisableCache;

extern int InitThreadGlobalStorage (void);
extern int CreateThreadGlobalStorage (void);

#endif



#endif
