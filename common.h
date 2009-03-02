/* This program is free software; you can redistribute it and/or modify
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
 */

/*! \file common.h
    \brief Odd definitions
*/

#ifndef _COMMON_H_
#define _COMMON_H_
#include "math.h"

#if !_GNU_SOURCE && !defined (__attribute__)
/*! \brief GNU C specific attributes, e.g. unused
// */
#define __attribute__(X)
#endif

/*! \brief Safe double error value (and float with a cast)
 */
#if defined(HUGE_VAL)
#define ERR_VAL (-HUGE_VAL)
#else
#define ERR_VAL (-FLT_MAX)
#endif

/*! \brief Macro to extract sign
 */
#ifndef SGN
#define SGN(x) ((x) > 0 ? 1 : -1)
#endif

/*! \brief typedef of signal handler function
 */
#if HAVE_SIGACTION
typedef struct sigaction psighandler;
#elif HAVE_SIGVEC
typedef struct sigvec psighandler;
#else
typedef RETSIGTYPE(*psighandler) (int);
#endif

#ifndef _MSC_VER
#define sqrtf(arg) (float)sqrt((double)(arg))
#define fabsf(arg) (float)fabs((double)(arg))
#endif
/* abs returns unsigned int by definition */
#define Abs(a) ((unsigned int)abs(a))

/* Do we need to use g_utf8_casefold() for utf8 anywhere? */
#define StrCaseCmp(s1, s2) g_ascii_strcasecmp(s1, s2)
#define StrNCaseCmp(s1, s2, n) g_ascii_strncasecmp(s1, s2, (gsize)n)
/* Avoid new code using strcase functions */
#define strcasecmp strcasecmp_error_use_StrCaseCmp
#define strncasecmp strncasecmp_error_use_StrNCaseCmp

#ifndef WIN32
#if defined(HAVE_SIGNBIT) && !HAVE_SIGNBIT
#define signbit(x) ((x) < 0.0)
#endif
#if defined(HAVE_LRINT) && !HAVE_LRINT
#define lrint(x) ((long) ((x)+0.5))
#endif
#endif
#endif

/* Macro to mark paramaters that aren't used in the function */	
#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(_lint)
# define UNUSED(x) /*lint -e{715}*/ _unused_##x
#else
# define UNUSED(x) _unused_##x
#endif
