/*
 * strdup.c
 *
 * by Gary Wong <gtw@gnu.org>, 2002
 *
 * $Id: strdup.c,v 1.2 2004/02/24 10:20:47 uid68519 Exp $
 */

#include <stdlib.h>
#include <string.h>
#include "config.h"

#if !HAVE_STRDUP
extern char *strdup( const char *sz ) {

    char *pch;
    size_t cch;
    
    if( !sz )
	return NULL;

    cch = strlen( sz );

    if( !( pch = malloc( cch + 1 ) ) )
	return NULL;

    memcpy( pch, sz, cch + 1 );

    return pch;
}
#endif
