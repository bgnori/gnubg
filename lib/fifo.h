/*
 * fifo.h
 *
 * by Gary Wong, 1996
 * $Id: fifo.h,v 1.3 2005/02/21 23:23:08 jsegrave Exp $
 *
 */

#ifndef _FIFO_H_
#define _FIFO_H_

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

typedef struct _fifo {
    int cbSize;
    char *pchBuffer;
    int iHead, cb;
} fifo;

extern int FifoCreate( fifo *pf, int cb );
extern int FifoDestroy( fifo *pf );

extern int FifoUsed( fifo *pf );
extern int FifoRemaining( fifo *pf );
extern int FifoCopyTo( fifo *pf, char *pch, int cch );
extern int FifoCopyFrom( fifo *pf, char *pch, int cch );
extern int FifoProduce( fifo *pf, int cch );
extern int FifoConsume( fifo *pf, int cch );
#if HAVE_READV
#define HAVE_FIFOIOVEC 1
extern int FifoIOVec( fifo *pf, struct iovec *piov, int fIn );
#endif

#endif
