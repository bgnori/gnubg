/*
 * onechequer.h
 *
 * by Joern Thyssen <jthyssen@dk.ibm.com>, 2002
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
 * $Id: onechequer.h,v 1.4 2004/01/03 13:35:08 uid65656 Exp $
 */

#ifndef _ONECHEQUER_H_
#define _ONECHEQUER_H_

extern float
GWCFromPipCount( const int anPips[ 2 ], float *arMu, float *arSigma );

extern float
GWCFromMuSigma( const float arMu[ 2 ], const float arSigma[ 2 ] );

#endif /* _ONECHEQUER_H_ */
