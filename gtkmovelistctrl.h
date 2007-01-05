/*
 * gtkmovelistctrl.h
 * by Jon Kinsey, 2005
 *
 * Analysis move list control
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
 * $Id: gtkmovelistctrl.h,v 1.2 2007/01/05 22:01:02 Superfly_Jon Exp $
 */

#ifndef _GTKMOVELISTCTRL_H_
#define _GTKMOVELISTCTRL_H_

#include <gtk/gtk.h>

#include "eval.h"

/* Some boilerplate GObject type check and type cast macros.
*  'klass' is used here instead of 'class', because 'class'
*  is a c++ keyword */

#define CUSTOM_TYPE_CELL_RENDERER_MOVELIST             (custom_cell_renderer_movelist_get_type())
#define CUSTOM_CELL_RENDERER_MOVELIST(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),  CUSTOM_TYPE_CELL_RENDERER_MOVELIST, CustomCellRendererMovelist))
#define CUSTOM_CELL_RENDERER_MOVELIST_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  CUSTOM_TYPE_CELL_RENDERER_MOVELIST, CustomCellRendererMovelistClass))
#define CUSTOM_IS_CELL_MOVELIST_MOVELIST(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CUSTOM_TYPE_CELL_RENDERER_MOVELIST))
#define CUSTOM_IS_CELL_MOVELIST_MOVELIST_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  CUSTOM_TYPE_CELL_RENDERER_MOVELIST))
#define CUSTOM_CELL_RENDERER_MOVELIST_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  CUSTOM_TYPE_CELL_RENDERER_MOVELIST, CustomCellRendererMovelistClass))

typedef struct _CustomCellRendererMovelist CustomCellRendererMovelist;
typedef struct _CustomCellRendererMovelistClass CustomCellRendererMovelistClass;

/* CustomCellRendererMovelist: Our custom cell renderer
*   structure. Extend according to need */

struct _CustomCellRendererMovelist
{
	GtkCellRenderer parent;
	move* pml;
	unsigned int rank;
};


struct _CustomCellRendererMovelistClass
{
	GtkCellRendererClass parent_class;
};


GType                custom_cell_renderer_movelist_get_type (void);
GtkCellRenderer     *custom_cell_renderer_movelist_new (void);
void custom_cell_renderer_invalidate_size();

#endif /* _custom_cell_renderer_movelistbar_included_ */
