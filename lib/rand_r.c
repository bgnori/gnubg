/*
 * rand_r.c
 *
 * by Gary Wong, 2000
 *
 * $Id: rand_r.c,v 1.1 2000/01/03 17:50:40 gtw Exp $
 */

extern unsigned int rand_r( unsigned int *pnSeed ) {

    /* A very simple rand_r function, for systems that don't have it.
       Provides poor quality random numbers, so don't use it for anything
       critical. */
    
    *pnSeed = *pnSeed * 1103515245 + 12345;

    return ( *pnSeed >> 16 ) & 0xFFFF;
}
