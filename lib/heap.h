/*
 * heap.h
 *
 * by Gary Wong, 1997
 * $Id: heap.h,v 1.2 2005/02/21 23:23:08 jsegrave Exp $
 */

#ifndef _HEAP_H_
#define _HEAP_H_

typedef int ( *heapcomparefunc )( void *p0, void *p1 );

typedef struct _heap {
    void **ap;
    int cp, cpAlloc;
    heapcomparefunc phcf;
} heap;

extern int HeapCreate( heap *ph, int c, heapcomparefunc phcf );
extern int HeapDestroy( heap *ph );
extern int HeapInsert( heap *ph, void *p );
extern void *HeapDelete( heap *ph );
extern int HeapDeleteItem( heap *ph, void *p );
extern void *HeapLookup( heap *ph );

#endif
