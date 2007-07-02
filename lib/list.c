/*
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
 * list.c
 *
 * by Gary Wong, 1996
 * $Id: list.c,v 1.10 2007/07/02 14:14:18 ace Exp $
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
