/*
* widget3d.c
* by Jon Kinsey, 2003
*
* GtkGLExt widget for 3d drawing
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
* $Id: widget3d.c,v 1.37 2007/12/12 23:08:22 Superfly_Jon Exp $
*/

#include "config.h"
#include "inc3d.h"
#include "misc3d.h"

gboolean gtk_gl_init_success = FALSE;

extern GdkGLConfig *getGlConfig(void)
{
	static GdkGLConfig *glconfig = NULL;
	if (!glconfig)
		glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE | GDK_GL_MODE_STENCIL);
	if (!glconfig)
	{
		glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);
		g_warning("Stencil buffer not available, no shadows\n");
	}
	if (!glconfig)
	{
		g_warning ("*** No appropriate OpenGL-capable visual found.\n");
	}
	return glconfig;
}

static gboolean configure_event_3d(GtkWidget *widget, GdkEventConfigure *notused, void* data)
{
	BoardData *bd = (BoardData*)data;
	if (display_is_3d(bd->rd))
	{
		static int curHeight = -1, curWidth = -1;
		int width = widget->allocation.width, height = widget->allocation.height;
		if (width != curWidth || height != curHeight)
		{
			GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

			if (!gdk_gl_drawable_gl_begin(gldrawable, gtk_widget_get_gl_context(widget)))
    			return FALSE;

			glViewport(0, 0, width, height);
			SetupViewingVolume3d(bd, bd->bd3d, bd->rd);

			RestrictiveRedraw();
			RerenderBase(bd->bd3d);

			gdk_gl_drawable_gl_end(gldrawable);
			
			curWidth = width, curHeight = height;
		}
	}
	return TRUE;
}

static void realize_3d(GtkWidget *widget, void* data)
{
	BoardData *bd = (BoardData*)data;
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
	if (!gdk_gl_drawable_gl_begin(gldrawable, gtk_widget_get_gl_context(widget)))
		return;

	InitGL(bd);
	GetTextures(bd->bd3d, bd->rd);
	preDraw3d(bd, bd->bd3d, bd->rd);
	/* Make sure viewing area is correct (in preview) */
	SetupViewingVolume3d(bd, bd->bd3d, bd->rd);

	if (fResetSync)
	{
		fResetSync = FALSE;
		(void)setVSync(fSync);
	}

	gdk_gl_drawable_gl_end(gldrawable);
}

void MakeCurrent3d(const BoardData3d *bd3d)
{
	GtkWidget *widget = bd3d->drawing_area3d;
	if (!gdk_gl_drawable_make_current(gtk_widget_get_gl_drawable(widget), gtk_widget_get_gl_context(widget)))
		g_print("gdk_gl_drawable_make_current failed!\n");
}

static gboolean expose_event_3d(GtkWidget *widget, GdkEventExpose *notused, const BoardData* bd)
{
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
	if (!gdk_gl_drawable_gl_begin(gldrawable, gtk_widget_get_gl_context(widget)))
		return TRUE;

	CheckOpenglError();

#ifndef TEST_HARNESS
	Draw3d(bd);
#else
	TestHarnessDraw(bd);
#endif

	gdk_gl_drawable_swap_buffers(gldrawable);

	gdk_gl_drawable_gl_end(gldrawable);

	return TRUE;
}

extern int CreateGLWidget(BoardData* bd)
{
	GtkWidget *p3dWidget;
	bd->bd3d = (BoardData3d*)malloc(sizeof(BoardData3d));
	InitBoard3d(bd, bd->bd3d);
	/* Drawing area for OpenGL */
	p3dWidget = bd->bd3d->drawing_area3d = gtk_drawing_area_new();

	/* Set OpenGL-capability to the widget - no list sharing */
	if (!gtk_widget_set_gl_capability(p3dWidget, getGlConfig(), NULL, TRUE, GDK_GL_RGBA_TYPE))
	{
		g_print("Can't create opengl capable widget\n");
		return FALSE;
	}

	if (p3dWidget == NULL)
	{
		g_print("Can't create opengl drawing widget\n");
		return FALSE;
	}
	/* set up events and signals for OpenGL widget */
	gtk_widget_set_events(p3dWidget, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
			GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_MOTION_MASK);
	g_signal_connect(G_OBJECT(p3dWidget), "button_press_event", G_CALLBACK(board_button_press), bd);
	g_signal_connect(G_OBJECT(p3dWidget), "button_release_event", G_CALLBACK(board_button_release), bd);
	g_signal_connect(G_OBJECT(p3dWidget), "motion_notify_event", G_CALLBACK(board_motion_notify), bd);
	g_signal_connect(G_OBJECT(p3dWidget), "realize", G_CALLBACK(realize_3d), bd);
	g_signal_connect(G_OBJECT(p3dWidget), "configure_event", G_CALLBACK(configure_event_3d), bd);
	g_signal_connect(G_OBJECT(p3dWidget), "expose_event", G_CALLBACK(expose_event_3d), bd);

	return TRUE;
}

void InitGTK3d(int *argc, char ***argv)
{
	gtk_gl_init_success = gtk_gl_init_check(argc, argv);
	/* Call LoadTextureInfo to get texture details from textures.txt */
	LoadTextureInfo();
}

/* This doesn't seem to work even on windows anymore... */
#ifdef TEMP_REMOVE

#ifndef PFD_GENERIC_ACCELERATED
#define PFD_GENERIC_ACCELERATED     0x00001000
#endif

void getFormatDetails(HDC hdc, int* accl, int* dbl, int* col, int* depth, int* stencil, int * accum)
{
	PIXELFORMATDESCRIPTOR pfd;

	/* Use current format */
	int format = GetPixelFormat(hdc);
	if (format == 0)
	{
		g_print("Unable to find pixel format.\n");
		return;
	}
	ZeroMemory(&pfd,sizeof(pfd));
	pfd.nSize=sizeof(pfd);
	pfd.nVersion=1;

	if (!DescribePixelFormat(hdc, format, sizeof(PIXELFORMATDESCRIPTOR), &pfd))
	{
		g_print("Unable to describe pixel format.\n");
		return;
	}

	*accl = !((pfd.dwFlags & PFD_GENERIC_FORMAT) && !(pfd.dwFlags & PFD_GENERIC_ACCELERATED));
	*dbl = pfd.dwFlags & PFD_DOUBLEBUFFER;
	*col = pfd.cColorBits;
	*depth = pfd.cDepthBits;
	*stencil = pfd.cStencilBits;
	*accum = pfd.cAccumBits;
}

static int CheckAccelerated(GtkWidget* board)
{
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(board);

	if (!gdk_gl_drawable_gl_begin(gldrawable, gtk_widget_get_gl_context(board)))
		return 1;
{
	int accl = 0, dbl, col, depth, stencil, accum;
	HDC dc = wglGetCurrentDC();

/* Display graphics card details
	HGLRC hglrc = wglGetCurrentContext();
	g_print("(hglrc:%p, dc:%p).\n", hglrc, dc);

	const char* vendor = glGetString(GL_VENDOR);
	const char* renderer = glGetString(GL_RENDERER);
	const char* version = glGetString(GL_VERSION);

	g_print("\n3d graphics card details\n\n");
	g_print("Vendor: %s\n", vendor);
	g_print("Renderer: %s\n", renderer);
	g_print("Version: %s\n", version);
*/

	if (!dc)
		g_print("No DC found.\n");
	else
	{
		getFormatDetails(dc, &accl, &dbl, &col, &depth, &stencil, &accum);
/*
		g_print("%sAccelerated\n(%s, %d bit col, %d bit depth, %d bit stencil, %d bit accum)\n",
			accl ? "" : "NOT ", dbl?"Double buffered":"Single buffered", col, depth, stencil, accum);
*/
	}
	gdk_gl_drawable_gl_end(gldrawable);

	return accl;
}
}

#else

static int CheckAccelerated(GtkWidget* notused)
{
/* Commented out check for non-windows systems,
	as doesn't work very well... */
	return TRUE;
/*
	Display* display = glXGetCurrentDisplay();
	GLXContext context = glXGetCurrentContext();
	if (!display || !context)
	{
		g_print("Unable to get current display information.\n");
		return 1;
	}
	return glXIsDirect(display, context);
*/
}

#endif

int DoAcceleratedCheck(const BoardData3d* bd3d, GtkWidget* pwParent)
{
	if (!CheckAccelerated(bd3d->drawing_area3d))
	{	/* Display warning message as performance may be bad */
		GTKShowWarning(WARN_UNACCELERATED, pwParent);
		return 0;
	}
	else
		return 1;
}

/* Drawing direct to pixmap */

static GdkGLContext *glPixmapContext = NULL;

static void SetupPreview(const BoardData* bd, renderdata* prd)
{
	GetTextures(bd->bd3d, prd);
	SetupViewingVolume3d(bd, bd->bd3d, prd);
	preDraw3d(bd, bd->bd3d, prd);
}

static GdkGLConfig *glconfigSingle = NULL;

extern GdkGLConfig *getglconfigSingle(void)
{
	if (!glconfigSingle)
		glconfigSingle = gdk_gl_config_new_by_mode((GdkGLConfigMode)(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_SINGLE | GDK_GL_MODE_STENCIL));
	if (!glconfigSingle)
		glconfigSingle = gdk_gl_config_new_by_mode((GdkGLConfigMode)(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_SINGLE));

	return glconfigSingle;
}

void *CreatePreviewBoard3d(const BoardData* bd, GdkPixmap *ppm)
{
	GdkGLPixmap *glpixmap = gdk_pixmap_set_gl_capability(ppm, getglconfigSingle(), NULL);
	GdkGLDrawable *gldrawable = GDK_GL_DRAWABLE(glpixmap);
	glPixmapContext = gdk_gl_context_new (gldrawable, NULL, FALSE, GDK_GL_RGBA_TYPE);

	if (!gdk_gl_drawable_gl_begin (gldrawable, glPixmapContext))
		return 0;

	InitGL(bd);

	gdk_gl_drawable_gl_end (gldrawable);

	return glpixmap;
}

void RenderBoard3d(const BoardData* bd, renderdata* prd, void *glpixmap, unsigned char* buf)
{
	GLint viewport[4];
	/*** OpenGL BEGIN ***/
	GdkGLDrawable *gldrawable = GDK_GL_DRAWABLE((GdkGLPixmap *)glpixmap);

	if (!gdk_gl_drawable_gl_begin (gldrawable, glPixmapContext))
		return;

	SetupPreview(bd, prd);

	Draw3d(bd);

	glGetIntegerv (GL_VIEWPORT, viewport);
	glReadPixels(0, 0, viewport[2], viewport[3], GL_RGB, GL_UNSIGNED_BYTE, buf);

	gdk_gl_drawable_gl_end(gldrawable);
	/*** OpenGL END ***/
}
