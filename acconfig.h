/*
 * acconfig.h
 *
 * by Gary Wong, 1999
 *
 * $Id: acconfig.h,v 1.3 2000/03/06 22:29:36 gtw Exp $
 */

/* Installation directory (used to determine PKGDATADIR below). */
#undef DATADIR

@BOTTOM@

/* The directory where the weights and databases will be stored. */
#define PKGDATADIR DATADIR "/" PACKAGE

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif
