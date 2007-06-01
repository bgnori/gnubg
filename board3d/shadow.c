/*
* shadow.c
* by Jon Kinsey, 2003
*
* 3d shadow functions
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
* $Id: shadow.c,v 1.13 2007/06/01 18:36:48 c_anthon Exp $
*/

#include "inc3d.h"

static int midStencilVal;

void shadowInit(BoardData3d *bd3d, renderdata *prd)
{
	int i;
	int stencilBits;

	/* Darkness as percentage of ambient light */
	prd->dimness = ((prd->lightLevels[1] / 100.0f) * (100 - prd->shadowDarkness)) / 100;

	for (i = 0; i < NUM_OCC; i++)
		bd3d->Occluders[i].handle = 0;

	/* Check the stencil buffer is present */
	glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
	if (!stencilBits)
	{
		g_print("No stencil buffer - no shadows\n");
		return;
	}
	midStencilVal = powi(2, stencilBits - 1);
	glClearStencil(midStencilVal);
}
#if 0  /* Used for testing */
void draw_shadow_volume_edges(Occluder* pOcc)
{
	if (pOcc->show)
	{
		glPushMatrix();
		moveToOcc(pOcc);
		GenerateShadowEdges(pOcc);
		glPopMatrix();
	}
}
#endif
static void draw_shadow_volume_extruded_edges(Occluder* pOcc, const float light_position[4], unsigned int prim)
{
	if (pOcc->show)
	{
		float olight[4];
		mult_matrix_vec(pOcc->invMat, light_position, olight);

		glPushMatrix();
		moveToOcc(pOcc);
		glBegin(prim);
		GenerateShadowVolume(pOcc, olight);
		glEnd();
		glPopMatrix();
	}
}

static void draw_shadow_volume_to_stencil(BoardData3d* bd3d)
{
	int i;

	/* First clear the stencil buffer */
	glClear(GL_STENCIL_BUFFER_BIT);

	/* Disable drawing to colour buffer, lighting and don't update depth buffer */
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);

	/* Z-pass approach */
	glStencilFunc(GL_ALWAYS, midStencilVal, (GLuint)~0);

	glCullFace(GL_FRONT);
	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);

	for (i = 0; i < NUM_OCC; i++)
		draw_shadow_volume_extruded_edges(&bd3d->Occluders[i], bd3d->shadow_light_position, GL_QUADS);

	glCullFace(GL_BACK);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

	for (i = 0; i < NUM_OCC; i++)
		draw_shadow_volume_extruded_edges(&bd3d->Occluders[i], bd3d->shadow_light_position, GL_QUADS);

	/* Enable colour buffer, lighting and depth buffer */
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_LIGHTING);
	glDepthMask(GL_TRUE);
}

void shadowDisplay(void (*drawScene)(const BoardData *, BoardData3d *, const renderdata *), const BoardData* bd, BoardData3d *bd3d, const renderdata *prd)
{
	/* Pass 1: Draw model, ambient light only (some diffuse to vary shadow darkness) */
	float zero[4] = {0,0,0,0};
	float d1[4];
	float specular[4];
	float diffuse[4];

	glGetLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	d1[0] = d1[1] = d1[2] = d1[3] = prd->dimness;
	glLightfv(GL_LIGHT0, GL_DIFFUSE, d1);
	/* No specular light */
	glGetLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_SPECULAR, zero);

	drawScene(bd, bd3d, prd);

	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	/* Create shadow volume in stencil buffer */
	glEnable(GL_STENCIL_TEST);
	draw_shadow_volume_to_stencil(bd3d);

	/* Pass 2: Redraw model, full light in non-shadowed areas */
	glStencilFunc(GL_EQUAL, midStencilVal, (GLuint)~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	drawScene(bd, bd3d, prd);

	glDisable(GL_STENCIL_TEST);
}
