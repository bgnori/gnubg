/*
 * gtklocdefs.h
 *
 * by Michael Petch <mpetch@capp-sysware.com>, 2011.
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
 * $Id: gtklocdefs.h,v 1.3 2011/09/02 21:48:56 mdpetch Exp $
 */

#ifndef _GTKLOCDEFS_H_
#define _GTKLOCDEFS_H_

#include "config.h"

#if (USE_GTK)
#include <gtk/gtk.h>

#if ! GTK_CHECK_VERSION(2,24,0)
#define  gtk_combo_box_text_new_with_entry gtk_combo_box_entry_new_text 
#endif

#if ! GTK_CHECK_VERSION(2,20,0)
#define gtk_widget_set_mapped(widget,fMap) \
	{ \
		if ((fMap)) \
			GTK_WIDGET_SET_FLAGS((widget), GTK_MAPPED); \
		else \
			GTK_WIDGET_UNSET_FLAGS((widget), GTK_MAPPED); \
	};

#define gtk_widget_get_realized(p)  GTK_WIDGET_REALIZED((p))
#define gtk_widget_has_grab(p)  GTK_WIDGET_HAS_GRAB((p))
#define gtk_widget_get_visible(p)  GTK_WIDGET_VISIBLE((p))
#define gtk_widget_get_mapped(p)  GTK_WIDGET_MAPPED((p))
#define gtk_widget_get_sensitive(p)  GTK_WIDGET_SENSITIVE((p))
#define gtk_widget_is_sensitive(p)  GTK_WIDGET_IS_SENSITIVE((p))
#define gtk_widget_has_focus(p)  GTK_WIDGET_HAS_FOCUS((p))
#endif

#if ! GTK_CHECK_VERSION(2,18,0)
#define gtk_widget_set_has_window(widget,has_window) \
	{ \
		if ((has_window)) \
			GTK_WIDGET_UNSET_FLAGS((widget), GTK_NO_WINDOW); \
		else \
			GTK_WIDGET_SET_FLAGS((widget), GTK_NO_WINDOW); \
	};

#define gtk_widget_set_can_focus(widget,can_focus) \
	{ \
		if ((can_focus)) \
			GTK_WIDGET_SET_FLAGS((widget), GTK_CAN_FOCUS); \
		else \
			GTK_WIDGET_UNSET_FLAGS((widget), GTK_CAN_FOCUS ); \
	};


extern void gtk_widget_get_allocation (GtkWidget *widget, GtkAllocation *allocation);
extern void gtk_widget_set_allocation (GtkWidget *widget, const GtkAllocation *allocation);
extern void gtk_cell_renderer_get_alignment (GtkCellRenderer *cell, gfloat *xalign, gfloat *yalign);
extern void gtk_cell_renderer_set_padding (GtkCellRenderer *cell, gint xpad, gint ypad);
#endif

#if ! GTK_CHECK_VERSION(2,14,0)

extern GtkWidget *gtk_dialog_get_action_area (GtkDialog *dialog);
extern GtkWidget *gtk_dialog_get_content_area (GtkDialog *dialog);
extern GtkWindow *gtk_widget_get_window (GtkWidget *widget);
extern gdouble gtk_adjustment_get_upper (GtkAdjustment *adjustment);
extern void gtk_adjustment_set_upper (GtkAdjustment *adjustment, gdouble upper);

#endif

#if ! GTK_CHECK_VERSION(2,12,0)
extern GtkTooltips *ptt;
#define gtk_widget_set_tooltip_text(pw,text) gtk_tooltips_set_tip(ptt, (pw), (text), NULL)
#endif

extern GtkWidget *get_statusbar_label (GtkStatusbar *statusbar);
extern void toolbar_set_orientation (GtkToolbar *toolbar, GtkOrientation orientation);

#ifdef GTK_DISABLE_DEPRECATED
#define USE_GTKUIMANAGER 1
#endif

#endif

#endif
