/*
 * gtkmovefilter.h
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
 * $Id: gtkmovefilter.h,v 1.2 2003/01/06 19:51:25 thyssen Exp $
 */

#ifndef _GTKMOVEFILTER_H_
#define _GTKMOVEFILTER_H_

extern GtkWidget *
MoveFilterWidget ( movefilter *pmf, 
                   int *pfOK,
                   GtkSignalFunc pfChanged, gpointer userdata );

extern void
SetMovefilterCommands ( const char *sz,
                  movefilter aamfNew[ MAX_FILTER_PLIES ][ MAX_FILTER_PLIES ], 
                  movefilter aamfOld[ MAX_FILTER_PLIES ][ MAX_FILTER_PLIES ] );

extern void
MoveFilterOK ( GtkWidget *pw, GtkWidget *pwMoveFilter );

extern void
MoveFilterSetPredefined ( GtkWidget *pwMoveFilter, 
                          const int i );

#endif /* _GTKMOVEFILTER_H_ */
