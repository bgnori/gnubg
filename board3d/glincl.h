/* $Id: glincl.h,v 1.2 2005/02/21 23:23:08 jsegrave Exp $ */
#if USE_BOARD3D

#if defined(WIN32) && (defined(__INTEL_COMPILER) || defined(_MSC_VER))
/* MS gl.h needs windows.h to be included first */
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#if !defined(WIN32)
/* x-windows include */
#include <GL/glx.h>
#endif

#endif
