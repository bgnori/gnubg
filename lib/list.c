/*
 * list.c
 *
 * by Gary Wong, 1996
 * $Id: list.c,v 1.9 2007/06/11 19:01:13 c_anthon Exp $
 */

#include "config.h"
#include "list.h"
#include <stddef.h>
#include <stdlib.h>

int ListCreate( list *pl ) {
    
    pl->plPrev = pl->plNext = pl;
    pl->p = NULL;

    return 0;
}

list *ListInsert( list *pl, void *p ) {

    list *plNew;

    if ( (plNew = (list*)malloc( sizeof( *plNew ))) == NULL )
		return NULL;

    plNew->p = p;

    plNew->plNext = pl;
    plNew->plPrev = pl->plPrev;

    pl->plPrev = plNew;
    plNew->plPrev->plNext = plNew;

    return plNew;
}

void ListDelete( list *pl ) {

    pl->plPrev->plNext = pl->plNext;
    pl->plNext->plPrev = pl->plPrev;

    free( pl );
}

void ListDeleteAll( const list *pl ) {

	while( pl->plNext->p )
	{
		free(pl->plNext->p);
		ListDelete( pl->plNext );
	}
}
