/*
* font3d.c
* by Jon Kinsey, 2005
*
* Draw 3d numbers using luxi font and freetype library
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
* $Id: font3d.c,v 1.2 2005/09/24 10:45:35 Superfly_Jon Exp $
*/

#include "config.h"
#include "glincl.h"
#include "inc3d.h"
#include "gtkboard.h"
#include <assert.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define FONT_SIZE (base_unit / 20.0f)
#define CUBE_FONT_SIZE (base_unit / 24.0f)

extern unsigned char auchLuxiSR[];
extern unsigned int cbLuxiSR;
extern unsigned char auchLuxiRB[];
extern unsigned int cbLuxiRB;

typedef struct _Point
{
	double data[3];
} Point;

typedef struct _Contour
{
	vector points;
} Contour;

typedef struct _Vectoriser
{
	vector contours;
	int numPoints;
} Vectoriser;

typedef struct _Tesselation
{
	vector points;
	GLenum meshType;
} Tesselation;

typedef struct _Mesh
{
	vector tesselations;
} Mesh;

void PopulateVectoriser(Vectoriser* pVect, FT_Outline* pOutline);
void TidyMemory(Vectoriser* pVect, Mesh* pMesh);
void PopulateContour(Contour* pContour, FT_Vector* points, char* pointTags, int numberOfPoints);
void PopulateMesh(Vectoriser* pVect, Mesh* pMesh);
int MakeGlyph(FT_Outline* pOutline, int list);

int CreateOGLFont(FT_Library ftLib, OGLFont *pFont, unsigned char *pBufferBytes, unsigned int bufferSizeInBytes, int pointSize, float scale)
{
	int i, j;
	FT_Face face;

	memset(pFont, 0, sizeof(OGLFont));
	pFont->scale = scale;

	if (FT_New_Memory_Face(ftLib, (FT_Byte *)pBufferBytes, bufferSizeInBytes, 0, &face))
		return 0;

	if (FT_Set_Char_Size(face, 0, pointSize * 64 /* 26.6 fractional points */, 0, 0))
		return 0;

	{	/* Find height of "1" */
		int glyphIndex = FT_Get_Char_Index(face, '1');
		if (!glyphIndex)
			return 0;

		if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_HINTING) ||
			(face->glyph->format != ft_glyph_format_outline))
			return 0;

		pFont->height = (float)face->glyph->metrics.height * scale / 64;
		pFont->advance = face->glyph->advance.x;
	}

	/* Create digit glyphs */
	pFont->glyphs = glGenLists(10);

	for (i = 0; i < 10; i++)
	{
		int glyphIndex = FT_Get_Char_Index(face, '0' + i);
		if (!glyphIndex)
			return 0;

		if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_HINTING))
			return 0;
		assert(face->glyph->format == ft_glyph_format_outline);

		if (!MakeGlyph(&face->glyph->outline, pFont->glyphs + i))
			return 0;

		/* Calculate kerning table */
		for (j = 0; j < 10; j++)
		{
			FT_Vector kernAdvance;
			int nextGlyphIndex = FT_Get_Char_Index(face, '0' + j);
			if (!nextGlyphIndex || FT_Get_Kerning(face, glyphIndex, nextGlyphIndex, ft_kerning_unfitted, &kernAdvance))
				return 0;
			pFont->kern[i][j] = kernAdvance.x;
		}
	}

	return !FT_Done_Face(face);
}

int BuildFont3d(BoardData* bd)
{
	FT_Library ftLib;
	if (FT_Init_FreeType(&ftLib))
		return 0;

	if (!CreateOGLFont(ftLib, &bd->numberFont, auchLuxiSR, cbLuxiSR, 24, FONT_SIZE))
		return 0;
	if (!CreateOGLFont(ftLib, &bd->cubeFont, auchLuxiRB, cbLuxiRB, 44, CUBE_FONT_SIZE))
		return 0;

	return !FT_Done_FreeType(ftLib);
}

int MakeGlyph(FT_Outline* pOutline, int list)
{
	Vectoriser vect;
	Mesh mesh;
	int index, point;

	PopulateVectoriser(&vect, pOutline);

	if ((VectorSize(&vect.contours) < 1) || (vect.numPoints < 3))
		return 0;

	glNewList(list, GL_COMPILE);

	/* Solid font */
	PopulateMesh(&vect, &mesh);

	for (index = 0; index < VectorSize(&mesh.tesselations); index++)
	{
		int pointIndex;
		Tesselation* subMesh = (Tesselation*)VectorGet(&mesh.tesselations, index);

		glBegin(subMesh->meshType);
		for (pointIndex = 0; pointIndex < VectorSize(&subMesh->points); pointIndex++)
		{
			Point* point = (Point*)VectorGet(&subMesh->points, pointIndex);

			assert(point->data[2] == 0);
			glVertex2f((float)point->data[0] / 64.0f, (float)point->data[1] / 64.0f);
		}
		glEnd();
	}

	/* Outline font - anti-alias edges */
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);

	for (index = 0; index < VectorSize(&vect.contours); index++)
	{
		Contour* contour = (Contour*)VectorGet(&vect.contours, index);

		glBegin( GL_LINE_LOOP);
		for (point = 0; point < VectorSize(&contour->points); point++)
		{
			Point* pPoint = VectorGet(&contour->points, point);
			glVertex2f((float)pPoint->data[0] / 64.0f, (float)pPoint->data[1] / 64.0f);
		}
		glEnd();
	}
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);

	glEndList();

	TidyMemory(&vect, &mesh);

	return 1;
}

int GetKern(OGLFont *pFont, char cur, char next)
{
	if (next)
		return pFont->kern[cur - '0'][next - '0'];
	else
		return 0;
}

extern float getTextLen3d(OGLFont *pFont, const char* str)
{
	int len = 0;
	while (*str)
	{
		len += pFont->advance + GetKern(pFont, str[0], str[1]);
		str++;
	}
	return len / 64.0f * pFont->scale;
}

extern int RenderString3d(OGLFont *pFont, const char* str)
{
	glScalef(pFont->scale, pFont->scale, 1);

	while (*str)
	{
		/* Draw character */
		glCallList(pFont->glyphs + (*str - '0'));

		/* Move on to next place */
		glTranslatef((pFont->advance + GetKern(pFont, str[0], str[1])) / 64.0f, 0, 0);

		str++;
	}
	return 1;
}

vector combineList;

void PopulateVectoriser(Vectoriser* pVect, FT_Outline* pOutline)
{
	int startIndex = 0;
	int endIndex;
	int contourLength;
	int contourIndex;

	VectorInit(&combineList, sizeof(double[4]));

	VectorInit(&pVect->contours, sizeof(Contour));
	pVect->numPoints = 0;

	for (contourIndex = 0; contourIndex < pOutline->n_contours; contourIndex++)
	{
		Contour newContour;
		FT_Vector* pointList = &pOutline->points[startIndex];
		char* tagList = &pOutline->tags[startIndex];

		endIndex = pOutline->contours[contourIndex];
		contourLength = (endIndex - startIndex) + 1;

		PopulateContour(&newContour, pointList, tagList, contourLength);
		pVect->numPoints += VectorSize(&newContour.points);
		VectorAdd(&pVect->contours, &newContour);

		startIndex = endIndex + 1;
	}
}

void AddPoint(Contour* pContour, double x, double y)
{
	if (VectorSize(&pContour->points) > 0)
	{	/* Ignore duplicate contour points */
		Point* point = (Point*)VectorGet(&pContour->points, VectorSize(&pContour->points) - 1);
		if (point->data[0] == x && point->data[1] == y)
			return;
	}

	{
		Point newPoint;
		newPoint.data[0] = x;
		newPoint.data[1] = y;
		newPoint.data[2] = 0;

		VectorAdd(&pContour->points, &newPoint);
	}
}

#define BEZIER_STEPS 5
#define BEZIER_STEP_SIZE 0.2f
double controlPoints[4][2]; /* 2D array storing values of de Casteljau algorithm */

void evaluateQuadraticCurve(Contour* pContour)
{
	int i;
	for (i = 0; i <= BEZIER_STEPS; i++)
	{
		double bezierValues[2][2];

		float t = i * BEZIER_STEP_SIZE;

		bezierValues[0][0] = (1.0f - t) * controlPoints[0][0] + t * controlPoints[1][0];
		bezierValues[0][1] = (1.0f - t) * controlPoints[0][1] + t * controlPoints[1][1];

		bezierValues[1][0] = (1.0f - t) * controlPoints[1][0] + t * controlPoints[2][0];
		bezierValues[1][1] = (1.0f - t) * controlPoints[1][1] + t * controlPoints[2][1];

		bezierValues[0][0] = (1.0f - t) * bezierValues[0][0] + t * bezierValues[1][0];
		bezierValues[0][1] = (1.0f - t) * bezierValues[0][1] + t * bezierValues[1][1];

		AddPoint(pContour, bezierValues[0][0], bezierValues[0][1]);
	}
}

void PopulateContour(Contour* pContour, FT_Vector* points, char* pointTags, int numberOfPoints)
{
	int pointIndex;

	VectorInit(&pContour->points, sizeof(Point));

	for (pointIndex = 0; pointIndex < numberOfPoints; pointIndex++)
	{
		char pointTag = pointTags[pointIndex];

		if (pointTag == FT_Curve_Tag_On || numberOfPoints < 2)
		{
			AddPoint(pContour, points[pointIndex].x, points[pointIndex].y);
		}
		else
		{
			int previousPointIndex = (pointIndex == 0) ? numberOfPoints - 1 : pointIndex - 1;
			int nextPointIndex = (pointIndex == numberOfPoints - 1) ? 0 : pointIndex + 1;
			Point controlPoint, previousPoint, nextPoint;
			controlPoint.data[0] = points[pointIndex].x;
			controlPoint.data[1] = points[pointIndex].y;
			previousPoint.data[0] = points[previousPointIndex].x;
			previousPoint.data[1] = points[previousPointIndex].y;
			nextPoint.data[0] = points[nextPointIndex].x;
			nextPoint.data[1] = points[nextPointIndex].y;

			assert(pointTag == FT_Curve_Tag_Conic);	/* Only this main type supported */

			while (pointTags[nextPointIndex] == FT_Curve_Tag_Conic)
			{
				nextPoint.data[0] = (controlPoint.data[0] + nextPoint.data[0]) * 0.5f;
				nextPoint.data[1] = (controlPoint.data[1] + nextPoint.data[1]) * 0.5f;

				controlPoints[0][0] = previousPoint.data[0];	controlPoints[0][1] = previousPoint.data[1];
				controlPoints[1][0] = controlPoint.data[0];		controlPoints[1][1] = controlPoint.data[1];
				controlPoints[2][0] = nextPoint.data[0];		controlPoints[2][1] = nextPoint.data[1];
				
				evaluateQuadraticCurve(pContour);

				pointIndex++;
				previousPoint = nextPoint;
				controlPoint.data[0] = points[pointIndex].x;
				controlPoint.data[1] = points[pointIndex].y;
				nextPointIndex = (pointIndex == numberOfPoints - 1) ? 0 : pointIndex + 1;
				nextPoint.data[0] = points[nextPointIndex].x;
				nextPoint.data[1] = points[nextPointIndex].y;
			}
			
			controlPoints[0][0] = previousPoint.data[0];	controlPoints[0][1] = previousPoint.data[1];
			controlPoints[1][0] = controlPoint.data[0];		controlPoints[1][1] = controlPoint.data[1];
			controlPoints[2][0] = nextPoint.data[0];		controlPoints[2][1] = nextPoint.data[1];
			
			evaluateQuadraticCurve(pContour);
		}
	}
}

/* Unfortunately the glu library doesn't define this callback type precisely
 so it may well cause problems on different platforms / opengl implementations */
#if WIN32
/* Need to set the callback calling convention for windows */
#define TESS_CALLBACK CALLBACK
#else
#define TESS_CALLBACK
#endif

Tesselation curTess;

void TESS_CALLBACK tcbError(GLenum errCode, Mesh* mesh)
{
	assert(0);	/* This is very unlikely to happen... */
}

void TESS_CALLBACK tcbVertex(void* data, Mesh* pMesh)
{
	double* vertex = (double*)data;
	Point newPoint;

	newPoint.data[0] = vertex[0];
	newPoint.data[1] = vertex[1];
	newPoint.data[2] = vertex[2];

	VectorAdd(&curTess.points, &newPoint);
}

void TESS_CALLBACK tcbCombine(double coords[3], void* vertex_data[4], GLfloat weight[4], void** outData, Mesh* pMesh)
{
	/* Just return vertex position (colours etc. not required) */
	VectorAdd(&combineList, coords);
	*outData = VectorGet(&combineList, VectorSize(&combineList) - 1);
}

void TESS_CALLBACK tcbBegin(GLenum type, Mesh* pMesh)
{
	VectorInit(&curTess.points, sizeof(Point));
	curTess.meshType = type;
}

void TESS_CALLBACK tcbEnd(Mesh* pMesh)
{
	VectorAdd(&pMesh->tesselations, &curTess);
}

void PopulateMesh(Vectoriser* pVect, Mesh* pMesh)
{
	int c, p;
	GLUtesselator* tobj = gluNewTess();

	VectorInit(&pMesh->tesselations, sizeof(Tesselation));

	gluTessCallback( tobj, GLU_TESS_BEGIN_DATA, tcbBegin);
	gluTessCallback( tobj, GLU_TESS_VERTEX_DATA, tcbVertex);
	gluTessCallback( tobj, GLU_TESS_COMBINE_DATA, tcbCombine);
	gluTessCallback( tobj, GLU_TESS_END_DATA, tcbEnd);
	gluTessCallback( tobj, GLU_TESS_ERROR_DATA, tcbError);

gluTessProperty( tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
//gluTessProperty( tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

	gluTessNormal( tobj, 0, 0, 1);

	gluTessBeginPolygon( tobj, pMesh);

	for(c = 0; c < VectorSize(&pVect->contours); ++c)
	{
		Contour* contour = (Contour*)VectorGet(&pVect->contours, c);

		gluTessBeginContour( tobj);

		for(p = 0; p < VectorSize(&contour->points); p++)
		{
			Point* point = (Point*)VectorGet(&contour->points, p);
			gluTessVertex(tobj, point->data, point->data);
		}

		gluTessEndContour( tobj);
	}

	gluTessEndPolygon( tobj);

	gluDeleteTess( tobj);
}

void TidyMemory(Vectoriser* pVect, Mesh* pMesh)
{
	int c, i;
	for (c = 0; c < VectorSize(&pVect->contours); c++)
	{
		Contour* contour = (Contour*)VectorGet(&pVect->contours, c);
		VectorClear(&contour->points);
	}
	VectorClear(&pVect->contours);
	for (i = 0; i < VectorSize(&pMesh->tesselations); i++)
	{
		Tesselation* subMesh = (Tesselation*)VectorGet(&pMesh->tesselations, i);
		VectorClear(&subMesh->points);
	}
	VectorClear(&pMesh->tesselations);

	VectorClear(&combineList);
}