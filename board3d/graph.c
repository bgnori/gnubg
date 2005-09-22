/*
* graph.c
* by Jon Kinsey, 2003
*
* Simple graph graphics
*
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
* $Id: graph.c,v 1.12 2005/09/22 17:50:08 Superfly_Jon Exp $
*/

#include <config.h>
#include "glincl.h"
#if HAVE_GTKGLEXT
#include <gtk/gtkgl.h>
#include <gtk/gtkglwidget.h>
#else
#include <gtkgl/gtkglarea.h>
#endif
#include <gtk/gtk.h>
#include <stdlib.h>
#include "inc3d.h"
#include "renderprefs.h"

#if HAVE_GTKGLEXT
extern GdkGLConfig *glconfig;
#endif

#define BAR_WIDTH 5
#define MID_GAP 1
#define INTER_GAP 4
#define TOTAL_GAP 5
#define RES_WIDTH (2 * BAR_WIDTH + MID_GAP + INTER_GAP)
#define NUM_WIDTH (modelWidth * NUM_WIDTH_PER)
#define NUM_WIDTH_PER .1f
#define NUM_HEIGHT (modelHeight * NUM_HEIGHT_PER)
#define NUM_HEIGHT_PER .15f
#define TOT_WIDTH (NUM_HEIGHT * 3)

float modelWidth, modelHeight;
BoardData fonts;
Texture total;

static gboolean graph_button_press_event(GtkWidget *board, GdkEventButton *event, void* arg)
{
	return FALSE;
}

static gboolean configure_event(GtkWidget *widget, GdkEventConfigure *notused, GraphData* gd)
{
	int width, height;
	float maxY, maxX;
#if HAVE_GTKGLEXT
	/*** OpenGL BEGIN ***/
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, gtk_widget_get_gl_context(widget)))
		return FALSE;
#else
	if (!gtk_gl_area_make_current(GTK_GL_AREA(widget)))
		return FALSE;
#endif
	width = widget->allocation.width;
	height = widget->allocation.height;

	maxX = (float)(gd->numGames * RES_WIDTH + RES_WIDTH + TOTAL_GAP);
	modelWidth = maxX * (1 + NUM_WIDTH_PER);

	maxY = gd->maxY * 1.05f + 1;
	modelHeight = maxY * (1 + NUM_HEIGHT_PER);

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, modelWidth, 0, modelHeight, -1, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

#if HAVE_GTKGLEXT
	gdk_gl_drawable_gl_end(gldrawable);
	/*** OpenGL END ***/
#endif

	return TRUE;
}

static void realize(GtkWidget *widget, void* arg)
{
#if HAVE_GTKGLEXT
	/*** OpenGL BEGIN ***/
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, gtk_widget_get_gl_context(widget)))
		return;
#else
    if (!gtk_gl_area_make_current(GTK_GL_AREA(widget)))
		return;
#endif
	/* Deep blue background colour */
	glClearColor(.2f, .2f, .4f, 1);

	BuildFont(&fonts);

	total.texID = 0;
	LoadTexture(&total, TEXTURE_PATH"total.bmp", TF_BMP);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if HAVE_GTKGLEXT
	gdk_gl_drawable_gl_end(gldrawable);
	/*** OpenGL END ***/
#endif
}

void DrawBar(float col[4], float x, float y, float first, float second)
{
	glPushMatrix();
	glTranslatef(x, y, 0);

	glBegin(GL_QUADS);
		glColor3f(0, 1, 0);
		glVertex2f(0, 0);
		glVertex2f(BAR_WIDTH, 0);
		glVertex2f(BAR_WIDTH, first);
		glVertex2f(0, first);

		glColor3f(0, 0, 1);
		glVertex2f(0, first);
		glVertex2f(BAR_WIDTH, first);
		glVertex2f(BAR_WIDTH, first + second);
		glVertex2f(0, first + second);
	glEnd();

	glLineWidth(3);
	glColor3f(col[0], col[1], col[2]);
	glBegin(GL_LINE_STRIP);
		glVertex2f(0, 0);
		glVertex2f(0, first + second);
		glVertex2f(BAR_WIDTH, first + second);
		glVertex2f(BAR_WIDTH, 0);
	glEnd();
	glLineWidth(1);
	glBegin(GL_POINTS);
		glVertex2f(0, first + second);
		glVertex2f(BAR_WIDTH, first + second);
	glEnd();

	glPopMatrix();
}

static void DrawColourBar(int player, float x, float y, float first, float second)
{
	float col[4];
	int i;
	renderdata *prd = GetMainAppearance();
	for (i = 0; i < 4; i++)
	{
		if (prd->fDisplayType == DT_2D)
			col[i] = (float)prd->aarColour[player][i];
		else
			col[i] = prd->ChequerMat[player].ambientColour[i];
	}
	DrawBar(col, x, y, first, second);
}

static void DrawBars(int num, float **values, int total)
{
	float x = NUM_WIDTH + RES_WIDTH * num;
	if (total)
		x += TOTAL_GAP;

	DrawColourBar(0, x + INTER_GAP / 2.0f, NUM_HEIGHT, values[0][0], values[0][1]);
	DrawColourBar(1, x + INTER_GAP / 2.0f + BAR_WIDTH + MID_GAP, NUM_HEIGHT, values[1][0], values[1][1]);
}

void PrintBottomNumber(int num, float width, float height, float x, float y)
{
	char numStr[10];
	sprintf(numStr, "%d", num);

	glPushMatrix();
	glTranslatef(x, y, 0);

	glColor3f(1, 1, 1);
	glScalef(width, height, 1);
	glLineWidth(.5f);
	glPrintCube(&fonts, numStr);
	glPopMatrix();
}

void PrintSideNumber(int num, float width, float height, float x, float y)
{
	char numStr[10];
	sprintf(numStr, "%d", num);

	glPushMatrix();
	glTranslatef(x, y, 0);

	glScalef(width, height, 1);
	glLineWidth(.5f);
	glPrintNumbersRA(&fonts, numStr);
	glPopMatrix();
}

void DrawLeftAxis(GraphData *pgd)
{
	int scale[] = {1, 5, 10, 20, 50, 100, 0};
	int* pScale = scale;
	int i, numPoints, pointInc;

	while (pScale[1] && pgd->maxY > *pScale * 5)
		pScale++;

	pointInc = *pScale;
	numPoints = (int)(pgd->maxY / pointInc);
	if (numPoints == 0)
		numPoints = 1;

	for (i = 1; i <= numPoints; i++)
	{
		float y = NUM_HEIGHT;
		y += i * pointInc;
		glColor3f(1, 1, 1);
		PrintSideNumber(i * pointInc, NUM_WIDTH * 10, NUM_HEIGHT * 10, NUM_WIDTH - 1, y);

		glColor3f(.5f, .5f, .5f);
		glLineStipple(1, 0xAAAA);
		glEnable(GL_LINE_STIPPLE);
		glBegin(GL_LINES);
			glVertex2f(NUM_WIDTH - .5f, y);
			glVertex2f(modelWidth - 1, y);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}
}

void DrawGraph(GraphData *gd)
{
	int i;
	float lastx = 0;

	if (total.texID)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, total.texID);

		glPushMatrix();
		glTranslatef(NUM_WIDTH + RES_WIDTH * gd->numGames + TOTAL_GAP + (INTER_GAP + MID_GAP) / 2.0f,
			NUM_HEIGHT / 2.0f - TOT_WIDTH / 6.0f, 0);

		glBegin(GL_QUADS);
			glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
			glTexCoord2f(1, 0); glVertex3f(BAR_WIDTH * 2, 0, 0);
			glTexCoord2f(1, 1); glVertex3f(BAR_WIDTH * 2, TOT_WIDTH, 0);
			glTexCoord2f(0, 1); glVertex3f(0, TOT_WIDTH, 0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	}

	DrawLeftAxis(gd);

	for (i = 0; i < gd->numGames; i++)
	{
		float x = NUM_WIDTH + RES_WIDTH * i + BAR_WIDTH + (INTER_GAP + MID_GAP) / 2.0f;

		DrawBars(i, gd->data[i], 0);

		if (x > lastx + NUM_WIDTH)
		{
			PrintBottomNumber(i + 1, NUM_WIDTH * 10, NUM_HEIGHT * 10, x, NUM_HEIGHT / 2.0f);
			lastx = x;
		}
	}

	/* Total bars */
	DrawBars(i, gd->data[i], 1);

	/* Axis */
	glColor3f(1, 1, 1);
	glBegin(GL_LINES);
		glVertex2f(NUM_WIDTH, NUM_HEIGHT);
		glVertex2f(modelWidth - 1, NUM_HEIGHT);
		glVertex2f(NUM_WIDTH, NUM_HEIGHT);
		glVertex2f(NUM_WIDTH, modelHeight * .95f);
	glEnd();
	glBegin(GL_POINTS);
		glVertex2f(NUM_WIDTH, NUM_HEIGHT);
	glEnd();
}

static gboolean expose_event(GtkWidget *widget, GdkEventExpose *event, GraphData* gd)
{
#if HAVE_GTKGLEXT
	/*** OpenGL BEGIN ***/
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, gtk_widget_get_gl_context(widget)))
		return TRUE;
#else

	/* OpenGL functions can be called only if make_current returns true */
	if (!gtk_gl_area_make_current(GTK_GL_AREA(widget)))
		return TRUE;
#endif
	CheckOpenglError();

	glClear(GL_COLOR_BUFFER_BIT);

	DrawGraph(gd);

#if HAVE_GTKGLEXT
	gdk_gl_drawable_swap_buffers(gldrawable);

	gdk_gl_drawable_gl_end(gldrawable);
	/*** OpenGL END ***/
#else
	gtk_gl_area_swapbuffers(GTK_GL_AREA(widget));
#endif

	return TRUE;
}

GtkWidget* StatGraph(GraphData* pgd)
{
	float f1, f2;
	GtkWidget* pw;
#if HAVE_GTKGLEXT
	/* Drawing area for OpenGL */
	pw = gtk_drawing_area_new();
	/* Set OpenGL-capability to the widget - no list sharing */
	gtk_widget_set_gl_capability(pw, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
#else
	pw = gtk_gl_area_new_vargs(NULL, /* no sharing of lists */
			 GDK_GL_RGBA, GDK_GL_DOUBLEBUFFER, GDK_GL_STENCIL_SIZE, 1, GDK_GL_NONE);
#endif
	if (pw == NULL)
	{
		g_print("Can't create opengl drawing widget\n");
		return 0;
	}

	f1 = pgd->data[pgd->numGames][0][0] + pgd->data[pgd->numGames][0][1];
	f2 = pgd->data[pgd->numGames][1][0] + pgd->data[pgd->numGames][1][1];
	pgd->maxY = (f1 > f2) ? f1 : f2;
	if (pgd->maxY < .5f)
		pgd->maxY = .5f;

	gtk_widget_set_events(pw, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect(GTK_OBJECT(pw), "button_press_event", GTK_SIGNAL_FUNC(graph_button_press_event), pgd);
	gtk_signal_connect(GTK_OBJECT(pw), "realize", GTK_SIGNAL_FUNC(realize), pgd);
	gtk_signal_connect(GTK_OBJECT(pw), "configure_event", GTK_SIGNAL_FUNC(configure_event), pgd);
	gtk_signal_connect(GTK_OBJECT(pw), "expose_event", GTK_SIGNAL_FUNC(expose_event), pgd);

	return pw;
}

void SetNumGames(GraphData* pgd, int numGames)
{
	pgd->numGames = numGames;
	pgd->data = Alloc3d(numGames + 1, 2, 2);
}

void TidyGraphData(GraphData* pgd)
{
	Free3d(pgd->data, pgd->numGames, 2);
}

void AddGameData(GraphData* pgd, int game, statcontext *psc)
{
	float aaaar[3][2][2][2];
	float f1, s1, f2, s2;
	getMWCFromError ( psc, aaaar );

	f1 = psc->arErrorCheckerplay[0][0] * 10;
	s1 = aaaar[ CUBEDECISION ][ TOTAL ][ PLAYER_0 ][0] * 10;
	f2 = psc->arErrorCheckerplay[1][0] * 10;
	s2 = aaaar[ CUBEDECISION ][ TOTAL ][ PLAYER_1 ][0] * 10;

	pgd->data[game][0][0] = f1;
	pgd->data[game][0][1] = s1;
	pgd->data[game][1][0] = f2;
	pgd->data[game][1][1] = s2;
}
