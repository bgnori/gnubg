/*
 * formatgs.h
 *
 * by Joern Thyssen <jth@gnubg.org>, 2003
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
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
 * $Id: formatgs.h,v 1.1 2003/08/30 18:10:01 thyssen Exp $
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
formatGS( const statcontext *psc, const matchstate *pms,
          const int fIsMatch, const enum _formatgs fg );

extern void
freeGS( GList *list );

#endif /* _FORMATGS_H_ */
