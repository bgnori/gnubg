/*
 * dynarray.h
 *
 * by Gary Wong, 1996
 * $Id: dynarray.h,v 1.3 2006/12/26 11:22:06 Superfly_Jon Exp $
 */

#ifndef _DYNARRAY_H_
#define _DYNARRAY_H_

typedef struct _dynarray {
    void **ap;
    unsigned int c, cp, iFinish;
    int fCompact;
} dynarray;

extern int DynArrayCreate( dynarray *pda, unsigned int c, int fCompact );
extern void DynArrayDestroy( const dynarray *pda );
extern unsigned int DynArrayAdd( dynarray *pda, void *p );
extern int DynArrayDelete( dynarray *pda, unsigned int i );
extern int DynArrayRemove( dynarray *pda, const void *p );
extern int DynArraySet( dynarray *pda, unsigned int i, void *p );

#endif
