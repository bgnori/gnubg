/*
 * gtkcolour.h
 *
 * by Gary Wong <gtw@gnu.org>, 2002.
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
 * $Id: gtkcolour.h,v 1.9 2007/09/14 11:30:48 c_anthon Exp $
 */

#ifndef _GTKCOLOUR_H_
#define _GTKCOLOUR_H_

#if USE_BOARD3D
#include "render.h"

#define DF_VARIABLE_OPACITY 1
#define DF_NO_ALPHA 2
#define DF_FULL_ALPHA 4

typedef struct UpdateDetails_T
{
	Material* pMat;
	GdkPixmap* pixmap;
	GtkWidget* preview;
	GtkWidget** parentPreview;
	int opacity;
	TextureType textureType;
} UpdateDetails;

GtkWidget* gtk_colour_picker_new3d(Material* pMat, int opacity, TextureType textureType);
#endif

#define GTK_TYPE_COLOUR_PICKER (gtk_colour_picker_get_type())
#define GTK_COLOUR_PICKER(obj) (GTK_CHECK_CAST((obj), GTK_TYPE_COLOUR_PICKER, \
	GtkColourPicker))
#define GTK_COLOUR_PICKER_CLASS(c) (GTK_CHECK_CLASS_CAST((c), \
	GTK_TYPE_COLOUR_PICKER, GtkColourPickerClass))
#define GTK_IS_COLOUR_PICKER(obj) (GTK_CHECK_TYPE((obj), \
	GTK_TYPE_COLOUR_PICKER))
#define GTK_IS_COLOUR_PICKER_CLASS(c) (GTK_CHECK_CLASS_TYPE((c), \
	GTK_TYPE_COLOUR_PICKER))
#define GTK_COLOUR_PICKER_GET_CLASS(obj) (GTK_CHECK_GET_CLASS((obj), \
	GTK_TYPE_COLOUR_PICKER, GtkColourPickerClass))

typedef struct _GtkColourPicker GtkColourPicker;
typedef struct _GtkColourPickerClass GtkColourPickerClass;

/*  Previous broken type  (YH)
    typedef GtkSignalFunc ColorPickerFunc;
*/
typedef void  (*ColorPickerFunc)(GtkWidget *);

struct _GtkColourPicker {
  GtkButton parent_instance;
  GtkWidget *pwColourSel, *pwDraw;
  GdkPixmap *ppm;
  gdouble arOrig[ 4 ];
  gdouble arColour[ 4 ];
  int hasOpacity;
  ColorPickerFunc  func;
  void *data;
};

struct _GtkColourPickerClass {
  GtkButtonClass parent_class;
};

extern GtkType
gtk_colour_picker_get_type( void );

extern GtkWidget*
gtk_colour_picker_new(ColorPickerFunc func, void *data);

extern void
gtk_colour_picker_set_has_opacity_control(GtkColourPicker *pcp, gboolean f );

extern void
gtk_colour_picker_set_colour( GtkColourPicker *pcp, gdouble *ar );

extern void
gtk_colour_picker_get_colour( GtkColourPicker *pcp, gdouble *ar );
    

#endif
