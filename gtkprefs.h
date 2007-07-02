/*
 * gtkprefs.h
 *
 * by Gary Wong <gtw@gnu.org>, 2000.
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
 * $Id: gtkprefs.h,v 1.12 2007/07/02 12:43:39 ace Exp $
 */

#ifndef _GTKPREFS_H_
#define _GTKPREFS_H_

#include "gtkboard.h"

extern void BoardPreferences( GtkWidget *pwBoard );
extern void BoardPreferencesStart( GtkWidget *pwBoard );
extern void BoardPreferencesDone( GtkWidget *pwBoard );
extern void Default3dSettings(BoardData* bd);
extern void UpdatePreview(GtkWidget *notused);

extern GtkWidget *pwPrevBoard;

#endif
