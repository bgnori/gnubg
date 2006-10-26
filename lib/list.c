/*
 * list.c
 *
 * by Gary Wong, 1996
 * $Id: list.c,v 1.7 2006/10/26 17:02:51 Superfly_Jon Exp $
 */

#include <list.h>
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

int ListDelete( list *pl ) {

    pl->plPrev->plNext = pl->plNext;
    pl->plNext->plPrev = pl->plPrev;

    free( pl );

    return 0;
}

int ListDeleteAll( list *pl ) {

	while( pl->plNext->p )
	{
		free(pl->plNext->p);
		ListDelete( pl->plNext );
	}

	return 0;
}
