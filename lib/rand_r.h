/*
 * rand_r.h
 *
 * by Gary Wong, 2000
 *
 * $Id: rand_r.h,v 1.2 2000/06/19 18:50:03 oysteijo Exp $
 */

/* Don't use this prototype in Cygwin32 , -Oystein */

#ifndef _RAND_R_H_
#ifndef __CYGWIN__
extern unsigned int rand_r( unsigned int *pnSeed );
#endif /* CYGWIN */
#endif
