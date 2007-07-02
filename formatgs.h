/*
 * formatgs.h
 *
 * by Joern Thyssen <jth@gnubg.org>, 2003
 *
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
 * $Id: formatgs.h,v 1.3 2007/07/02 12:43:38 ace Exp $
 */

#ifndef _FORMATGS_H_
#define _FORMATGS_H_

#include <glib.h>

#include "analysis.h"

enum _formatgs {
  FORMATGS_CHEQUER,
  FORMATGS_CUBE, 
  FORMATGS_LUCK,
  FORMATGS_OVERALL };

extern GList *
formatGS( const statcontext *psc, const int nMatchTo,
          const int fIsMatch, const enum _formatgs fg );

extern void
freeGS( GList *list );

#endif /* _FORMATGS_H_ */
