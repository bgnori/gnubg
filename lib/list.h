/*
 * list.h
 *
 * by Gary Wong, 1996
 * $Id: list.h,v 1.5 2006/12/26 11:22:06 Superfly_Jon Exp $
 */

#ifndef _LIST_H_
#define _LIST_H_

typedef struct _list {
  struct _list* plPrev;
  struct _list* plNext;
  void* p;
} list;

extern int ListCreate( list *pl );
/* #define ListDestroy( pl ) ( assert( ListEmpty( pl ) ) ) */

#define ListEmpty( pl ) ( (pl)->plNext == (pl) )
extern list* ListInsert( list* pl, void* p );
extern void ListDelete( list* pl );
extern void ListDeleteAll( const list *pl );

#endif
