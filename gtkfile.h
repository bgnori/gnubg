/*
 * gtkfile.c
 *
 * by Christian Anthon 2006
 *
 * File dialogs
 *
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
 * $Id: gtkfile.h,v 1.7 2008/04/27 20:18:48 c_anthon Exp $
 */

#ifndef _GTKFILE_H_
#define _GTKFILE_H_
extern void GTKOpen (gpointer p, guint n, GtkWidget * pw);
extern void GTKSave (gpointer p, guint n, GtkWidget * pw);
extern char *GTKFileSelect (const gchar * prompt, const gchar * extension, const gchar * folder,
			    const gchar * name, GtkFileChooserAction action);
extern void SetDefaultFileName (char *path);
extern void GTKBatchAnalyse( gpointer p, guint n, GtkWidget *pw);
#endif
