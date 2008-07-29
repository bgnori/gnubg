/*
 * renderprefs.c
 *
 * by Gary Wong <gtw@gnu.org>, 2000, 2001, 2002, 2003.
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
 * $Id: renderprefs.c,v 1.39 2008/07/29 11:46:35 c_anthon Exp $
 */

#include "config.h"
#include "backgammon.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "render.h"
#include "renderprefs.h"
#if USE_GTK
#include <gtk/gtk.h>
#include "gtkboard.h"
#include "gtkgame.h"
#endif
#if USE_BOARD3D
#include "fun3d.h"
#endif

const char *aszWoodName[] = {
  "alder", "ash", "basswood", "beech", "cedar", "ebony", "fir", "maple",
  "oak", "pine", "redwood", "walnut", "willow", "paint"
};

renderdata rdAppearance;

/* Limit use of global... */
extern renderdata *
GetMainAppearance (void)
{
  return &rdAppearance;
}

extern void
CopyAppearance (renderdata * prd)
{
  memcpy (prd, &rdAppearance, sizeof (rdAppearance));
}

#if USE_GTK
static char
HexDigit (char ch)
{

  ch = toupper (ch);

  if (ch >= '0' && ch <= '9')
    return ch - '0';
  else if (ch >= 'A' && ch <= 'F')
    return ch - 'A' + 0xA;
  else
    return 0;
}

static int
SetColour (const char *sz, unsigned char anColour[])
{

  int i;

  if (strlen (sz) < 7 || *sz != '#')
    return -1;

  sz++;

  for (i = 0; i < 3; i++) {
    anColour[i] = (HexDigit (sz[0]) << 4) | HexDigit (sz[1]);
    sz += 2;
  }

  return 0;
}

static int
SetColourSpeckle (const char *sz, unsigned char anColour[], int *pnSpeckle)
{
  gchar **strs = g_strsplit( sz, ";", 0);
	  
  if (SetColour (strs[0], anColour))
	  return -1;

  *pnSpeckle = (int) (g_ascii_strtod (strs[1], NULL) * 128);

  g_strfreev(strs);
  
  if (*pnSpeckle < 0)
    *pnSpeckle = 0;
  else if (*pnSpeckle > 128)
    *pnSpeckle = 128;

  return 0;
}

/* Set colour (with floats) */
static int
SetColourX (gdouble arColour[4], const char *sz)
{

  char *pch;
  unsigned char anColour[3];

  if ((pch = strchr (sz, ';')))
    *pch++ = 0;

  if (!SetColour (sz, anColour)) {
    arColour[0] = anColour[0] / 255.0f;
    arColour[1] = anColour[1] / 255.0f;
    arColour[2] = anColour[2] / 255.0f;
    return 0;
  }

  return -1;
}

#if USE_BOARD3D
static int
SetColourF (float arColour[4], const char *sz)
{

  char *pch;
  unsigned char anColour[3];

  if ((pch = strchr (sz, ';')))
    *pch++ = 0;

  if (!SetColour (sz, anColour)) {
    arColour[0] = anColour[0] / 255.0f;
    arColour[1] = anColour[1] / 255.0f;
    arColour[2] = anColour[2] / 255.0f;
    return 0;
  }

  return -1;
}
#endif /* USE_BOARD3D */
#endif /* USE_GTK */

#if USE_BOARD3D
static int
SetMaterialCommon (Material * pMat, const char *sz, const char **arg)
{
  float opac;
  char *pch;

  if (SetColourF (pMat->ambientColour, sz) != 0)
    return -1;
  sz += strlen (sz) + 1;

  if (SetColourF (pMat->diffuseColour, sz) != 0)
    return -1;
  sz += strlen (sz) + 1;

  if (SetColourF (pMat->specularColour, sz) != 0)
    return -1;

  if (sz)
    sz += strlen (sz) + 1;
  if ((pch = strchr (sz, ';')))
    *pch = 0;

  if (sz)
    pMat->shine = atoi (sz);
  else
    pMat->shine = 128;

  if (sz)
    sz += strlen (sz) + 1;
  if ((pch = strchr (sz, ';')))
    *pch = 0;
  if (sz) {
    int o = atoi (sz);
    if (o == 100)
      opac = 1;
    else
      opac = o / 100.0f;
  }
  else
    opac = 1;

  pMat->ambientColour[3] = pMat->diffuseColour[3] = pMat->specularColour[3] =
    opac;
  pMat->alphaBlend = (opac != 1) && (opac != 0);

  if (pch) {
    sz += strlen (sz) + 1;
    if (sz && *sz) {
      *arg = sz;
      return 1;
    }
  }
  return 0;
}

static int
SetMaterial (Material * pMat, const char *sz)
{
  int ret = 0;
  if (fX) {
    const char *arg;
    ret = SetMaterialCommon (pMat, sz, &arg);
    pMat->textureInfo = 0;
    pMat->pTexture = 0;
    if (ret > 0) {
      FindTexture (&pMat->textureInfo, arg);
      ret = 0;
    }
  }
  return ret;
}

static int
SetMaterialDice (Material * pMat, const char *sz, int *flag)
{
  const char *arg;
  int ret = SetMaterialCommon (pMat, sz, &arg);
  /* die colour same as chequer colour */
  *flag = TRUE;
  if (ret > 0) {
    *flag = (toupper (*arg) == 'Y');
    ret = 0;
  }
  return ret;
}

#endif

#if USE_GTK
/* Set colour, alpha, refraction, shine, specular. */
static int
SetColourARSS (double aarColour[2][4],
         float arRefraction[2],
         float arCoefficient[2], float arExponent[2], char *sz, int i)
{

  char *pch;

  if ((pch = strchr (sz, ';')))
    *pch++ = 0;

  if (!SetColourX (aarColour[i], sz)) {

    if (pch) {
      /* alpha */
      aarColour[i][3] = g_ascii_strtod (pch, NULL);

      if ((pch = strchr (pch, ';')))
  *pch++ = 0;
    }
    else
      aarColour[i][3] = 1.0f;  /* opaque */

    if (pch) {
      /* refraction */
      arRefraction[i] = g_ascii_strtod (pch, NULL);

      if ((pch = strchr (pch, ';')))
  *pch++ = 0;
    }
    else
      arRefraction[i] = 1.5f;

    if (pch) {
      /* shine */
      arCoefficient[i] = g_ascii_strtod (pch, NULL);

      if ((pch = strchr (pch, ';')))
  *pch++ = 0;
    }
    else
      arCoefficient[i] = 0.5f;

    if (pch) {
      /* specular */
      arExponent[i] = g_ascii_strtod (pch, NULL);

      if ((pch = strchr (pch, ';')))
  *pch++ = 0;
    }
    else
      arExponent[i] = 10.0f;

    return 0;
  }

  return -1;
}

/* Set colour, shine, specular, flag. */
static int
SetColourSSF (gdouble aarColour[2][4],
        gfloat arCoefficient[2],
        gfloat arExponent[2], int afDieColour[2], char *sz, int i)
{

  char *pch;

  if ((pch = strchr (sz, ';')))
    *pch++ = 0;

  if (!SetColourX (aarColour[i], sz)) {

    if (pch) {
      /* shine */
      arCoefficient[i] = (gfloat) g_ascii_strtod (pch, NULL);

      if ((pch = strchr (pch, ';')))
  *pch++ = 0;
    }
    else
      arCoefficient[i] = 0.5f;

    if (pch) {
      /* specular */
      arExponent[i] = (gfloat) g_ascii_strtod (pch, NULL);

      if ((pch = strchr (pch, ';')))
  *pch++ = 0;
    }
    else
      arExponent[i] = 10.0f;

    if (pch) {
      /* die colour same as chequer colour */
      afDieColour[i] = (toupper (*pch) == 'Y');

      if ((pch = strchr (pch, ';')))
  *pch++ = 0;
    }
    else
      afDieColour[i] = TRUE;

    return 0;
  }

  return -1;
}

static int
SetWood (const char *sz, woodtype * pbw)
{

  woodtype bw;
  int cch = strlen (sz);

  for (bw = 0; bw <= WOOD_PAINT; bw++)
    if (!StrNCaseCmp (sz, aszWoodName[bw], cch)) {
      *pbw = bw;
      return 0;
    }

  return -1;
}
#endif

#if USE_BOARD3D
static displaytype check_for_board3d (char *szValue)
{
	if (*szValue == '2')
		return DT_2D;
	else if ( !fX || gtk_gl_init_success)
		return DT_3D;
	return DT_2D;
}
#endif
extern void
RenderPreferencesParam (renderdata * prd, const char *szParam, char *szValue)
{
#if USE_GTK
int c, fValueError = FALSE;

  if (!szParam || !*szParam)
    return;

  c = strlen (szParam);

  if (!StrNCaseCmp (szParam, "board", c))
    /* board=colour;speckle */
    fValueError = SetColourSpeckle (szValue, prd->aanBoardColour[0],
            &prd->aSpeckle[0]);
  else if (!StrNCaseCmp (szParam, "border", c))
    /* border=colour */
    fValueError = SetColour (szValue, prd->aanBoardColour[1]);
  else if (!StrNCaseCmp (szParam, "cube", c))
    /* cube=colour */
    fValueError = SetColourX (prd->arCubeColour, szValue);
  else if (!StrNCaseCmp (szParam, "translucent", c))
    /* deprecated option "translucent"; ignore */
    ;
  else if (!StrNCaseCmp (szParam, "labels", c))
    /* labels=bool */
    prd->fLabels = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "dynamiclabels", c))
    /* dynamiclabels=bool */
    prd->fDynamicLabels = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "diceicon", c))
    /* FIXME deprecated in favour of "set gui dicearea" */
    prd->fDiceArea = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "show_ids", c))
    /* FIXME deprecated in favour of "set gui showids" */
    prd->fShowIDs = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "show_pips", c))
    /* FIXME deprecated in favour of "set gui showpips" */
    fGUIShowPips = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "illegal", c))
    /* FIXME deprecated in favour of "set gui illegal" */
    fGUIIllegal = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "beep", c))
    /* FIXME deprecated in favour of "set gui beep" */
    fGUIBeep = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "highdie", c))
    /* FIXME deprecated in favour of "set gui highdie" */
    fGUIHighDieFirst = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "wood", c))
    fValueError = SetWood (szValue, &prd->wt);
  else if (!StrNCaseCmp (szParam, "hinges", c))
    prd->fHinges = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "animate", c)) {
    /* FIXME deprecated in favour of "set gui animation ..." */
    switch (toupper (*szValue)) {
    case 'B':
      animGUI = ANIMATE_BLINK;
      break;
    case 'S':
      animGUI = ANIMATE_SLIDE;
      break;
    default:
      animGUI = ANIMATE_NONE;
      break;
    }
  }
  else if (!StrNCaseCmp (szParam, "speed", c)) {
    /* FIXME deprecated in favour of "set gui animation speed" */
    int n = atoi (szValue);

    if (n < 0 || n > 7)
      fValueError = TRUE;
    else
      nGUIAnimSpeed = n;
  }
  else if (!StrNCaseCmp (szParam, "light", c)) {
    /* light=azimuth;elevation */
    float rAzimuth, rElevation;
    gchar **split = g_strsplit( szValue, ";", 0);
    
    rAzimuth = (float) g_ascii_strtod (split[0], NULL);
    rElevation = (float) g_ascii_strtod (split[1], NULL);

    g_strfreev (split);

    if (rElevation < 0.0f)
      rElevation = 0.0f;
    else if (rElevation > 90.0f)
      rElevation = 90.0f;

    prd->arLight[2] = sinf (rElevation / 180 * G_PI);
    prd->arLight[0] = cosf (rAzimuth / 180 * G_PI) *
      sqrt (1.0 - prd->arLight[2] * prd->arLight[2]);
    prd->arLight[1] = sinf (rAzimuth / 180 * G_PI) *
      sqrt (1.0 - prd->arLight[2] * prd->arLight[2]);
  }
  else if (!StrNCaseCmp (szParam, "shape", c)) {
    float rRound = (float) g_ascii_strtod (szValue, &szValue);

    prd->rRound = 1.0 - rRound;
  }
  else if (!StrNCaseCmp (szParam, "moveindicator", c))
    prd->showMoveIndicator = toupper (*szValue) == 'Y';
#if USE_BOARD3D
  else if (!StrNCaseCmp (szParam, "boardtype", c))
     prd->fDisplayType = check_for_board3d(szValue);
  else if (!StrNCaseCmp (szParam, "hinges3d", c))
    prd->fHinges3d = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "boardshadows", c))
    prd->showShadows = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "shadowdarkness", c))
    prd->shadowDarkness = atoi (szValue);
  else if (!StrNCaseCmp (szParam, "animateroll", c))
    prd->animateRoll = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "animateflag", c))
    prd->animateFlag = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "closeboard", c))
    prd->closeBoardOnExit = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "quickdraw", c))
    prd->quickDraw = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "curveaccuracy", c))
    prd->curveAccuracy = atoi (szValue);
  else if (!StrNCaseCmp (szParam, "lighttype", c))
    prd->lightType = *szValue == 'p' ? LT_POSITIONAL : LT_DIRECTIONAL;
  else if (!StrNCaseCmp (szParam, "lightposx", c))
    prd->lightPos[0] = (float) g_ascii_strtod (szValue, &szValue);
  else if (!StrNCaseCmp (szParam, "lightposy", c))
    prd->lightPos[1] = (float) g_ascii_strtod (szValue, &szValue);
  else if (!StrNCaseCmp (szParam, "lightposz", c))
    prd->lightPos[2] = (float) g_ascii_strtod (szValue, &szValue);
  else if (!StrNCaseCmp (szParam, "lightambient", c))
    prd->lightLevels[0] = atoi (szValue);
  else if (!StrNCaseCmp (szParam, "lightdiffuse", c))
    prd->lightLevels[1] = atoi (szValue);
  else if (!StrNCaseCmp (szParam, "lightspecular", c))
    prd->lightLevels[2] = atoi (szValue);
  else if (!StrNCaseCmp (szParam, "boardangle", c))
    prd->boardAngle = atoi (szValue);
  else if (!StrNCaseCmp (szParam, "skewfactor", c))
    prd->skewFactor = atoi (szValue);
  else if (!StrNCaseCmp (szParam, "planview", c))
    prd->planView = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "dicesize", c))
    prd->diceSize = (float) g_ascii_strtod (szValue, &szValue);
  else if (!StrNCaseCmp (szParam, "roundededges", c))
    prd->roundedEdges = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "bgintrays", c))
    prd->bgInTrays = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "roundedpoints", c))
    prd->roundedPoints = toupper (*szValue) == 'Y';
  else if (!StrNCaseCmp (szParam, "piecetype", c))
    prd->pieceType = (PieceType) atoi (szValue);
  else if (!StrNCaseCmp (szParam, "piecetexturetype", c))
    prd->pieceTextureType = (PieceTextureType) atoi (szValue);
  else if ((!StrNCaseCmp (szParam, "chequers3d", strlen ("chequers3d")) ||
      !StrNCaseCmp (szParam, "checkers3d", strlen ("checkers3d"))) &&
     (szParam[c - 1] == '0' || szParam[c - 1] == '1'))
    fValueError =
      SetMaterial (&prd->ChequerMat[szParam[c - 1] - '0'], szValue);
  else if (!StrNCaseCmp (szParam, "dice3d", strlen ("dice3d"))
     && (szParam[c - 1] == '0' || szParam[c - 1] == '1'))
    fValueError =
      SetMaterialDice (&prd->DiceMat[szParam[c - 1] - '0'], szValue,
           &prd->afDieColour3d[szParam[c - 1] - '0']);
  else if (!StrNCaseCmp (szParam, "dot3d", strlen ("dot3d"))
     && (szParam[c - 1] == '0' || szParam[c - 1] == '1'))
    fValueError =
      SetMaterial (&prd->DiceDotMat[szParam[c - 1] - '0'], szValue);
  else if (!StrNCaseCmp (szParam, "cube3d", c))
    fValueError = SetMaterial (&prd->CubeMat, szValue);
  else if (!StrNCaseCmp (szParam, "cubetext3d", c))
    fValueError = SetMaterial (&prd->CubeNumberMat, szValue);
  else if (!StrNCaseCmp (szParam, "base3d", c))
    fValueError = SetMaterial (&prd->BaseMat, szValue);
  else if (!StrNCaseCmp (szParam, "points3d", strlen ("points3d")) &&
     (szParam[c - 1] == '0' || szParam[c - 1] == '1'))
    fValueError = SetMaterial (&prd->PointMat[szParam[c - 1] - '0'], szValue);
  else if (!StrNCaseCmp (szParam, "border3d", c))
    fValueError = SetMaterial (&prd->BoxMat, szValue);
  else if (!StrNCaseCmp (szParam, "hinge3d", c))
    fValueError = SetMaterial (&prd->HingeMat, szValue);
  else if (!StrNCaseCmp (szParam, "numbers3d", c))
    fValueError = SetMaterial (&prd->PointNumberMat, szValue);
  else if (!StrNCaseCmp (szParam, "background3d", c))
    fValueError = SetMaterial (&prd->BackGroundMat, szValue);
#endif
  else if (c > 1 &&
     (!StrNCaseCmp (szParam, "chequers", c - 1) ||
      !StrNCaseCmp (szParam, "checkers", c - 1)) &&
     (szParam[c - 1] == '0' || szParam[c - 1] == '1'))
    /* chequers=colour;alpha;refrac;shine;spec */
    fValueError = SetColourARSS (prd->aarColour,
         prd->arRefraction,
         prd->arCoefficient,
         prd->arExponent,
         szValue, szParam[c - 1] - '0');
  else if (c > 1 &&
     (!StrNCaseCmp (szParam, "dice", c - 1) ||
      !StrNCaseCmp (szParam, "dice", c - 1)) &&
     (szParam[c - 1] == '0' || szParam[c - 1] == '1'))
    /* dice=colour;shine;spec;flag */
    fValueError = SetColourSSF (prd->aarDiceColour,
        prd->arDiceCoefficient,
        prd->arDiceExponent,
        prd->afDieColour,
        szValue, szParam[c - 1] - '0');
  else if (c > 1 &&
     (!StrNCaseCmp (szParam, "dot", c - 1) ||
      !StrNCaseCmp (szParam, "dot", c - 1)) &&
     (szParam[c - 1] == '0' || szParam[c - 1] == '1'))
    /* dot=colour */
    fValueError =
      SetColourX (prd->aarDiceDotColour[szParam[c - 1] - '0'], szValue);
  else if (c > 1 && !StrNCaseCmp (szParam, "points", c - 1) &&
     (szParam[c - 1] == '0' || szParam[c - 1] == '1'))
    /* pointsn=colour;speckle */
    fValueError = SetColourSpeckle (szValue,
            prd->aanBoardColour[szParam[c - 1] - '0' +
              2],
            &prd->aSpeckle[szParam[c - 1] - '0' + 2]);

  if (fValueError)
    outputf (_("`%s' is not a legal value for parameter `%s'.\n"), szValue,
       szParam);

#endif

}

#if USE_BOARD3D

char *
WriteMaterial (Material * pMat)
{
  /* Bit of a hack to remove memory allocation worries */
#define NUM_MATS 20
  static int cur = 0;
  static char buf[NUM_MATS][100];
  cur = (cur + 1) % NUM_MATS;

  sprintf (buf[cur], "#%02X%02X%02X;#%02X%02X%02X;#%02X%02X%02X;%d;%d",
     (int) (pMat->ambientColour[0] * 0xFF),
     (int) (pMat->ambientColour[1] * 0xFF),
     (int) (pMat->ambientColour[2] * 0xFF),
     (int) (pMat->diffuseColour[0] * 0xFF),
     (int) (pMat->diffuseColour[1] * 0xFF),
     (int) (pMat->diffuseColour[2] * 0xFF),
     (int) (pMat->specularColour[0] * 0xFF),
     (int) (pMat->specularColour[1] * 0xFF),
     (int) (pMat->specularColour[2] * 0xFF),
     pMat->shine, (int) ((pMat->ambientColour[3] + .001f) * 100));
  if (pMat->textureInfo) {
    strcat (buf[cur], ";");
    strcat (buf[cur], MaterialGetTextureFilename(pMat));
  }
  return buf[cur];
}

char *
WriteMaterialDice (renderdata * prd, int num)
{
  char *buf = WriteMaterial (&prd->DiceMat[num]);
  strcat (buf, ";");
  strcat (buf, prd->afDieColour3d[num] ? "y" : "n");
  return buf;
}

#endif
extern GString *RenderPreferencesCommand(renderdata * prd)
{

	gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
	gchar buf1[G_ASCII_DTOSTR_BUF_SIZE];
	gchar buf2[G_ASCII_DTOSTR_BUF_SIZE];
	gchar buf3[G_ASCII_DTOSTR_BUF_SIZE];
	GString *gst = g_string_new(NULL);

	float rElevation = (float) (asinf(prd->arLight[2]) * 180 / G_PI);
	float rAzimuth = (fabs(prd->arLight[2] - 1.0f) < 1e-5) ? 0.0f :
	    (float) (acosf(prd->arLight[0] / sqrt(1.0 - prd->arLight[2] * prd->arLight[2])) * 180 / G_PI);

	if (prd->arLight[1] < 0)
		rAzimuth = 360 - rAzimuth;

	g_ascii_formatd(buf, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->aSpeckle[0] / 128.0f);
	g_string_append_printf(gst, "set appearance board=#%02X%02X%02X;%s ", prd->aanBoardColour[0][0],
			       prd->aanBoardColour[0][1], prd->aanBoardColour[0][2], buf);

	g_string_append_printf(gst, "border=#%02X%02X%02X ", prd->aanBoardColour[1][0], prd->aanBoardColour[1][1],
			       prd->aanBoardColour[1][2]);
	g_string_append_printf(gst, "moveindicator=%c ", prd->showMoveIndicator ? 'y' : 'n');

#if USE_BOARD3D
	g_string_append_printf(gst, "boardtype=%c ", display_is_2d(prd) ? '2' : '3');
	g_string_append_printf(gst, "hinges3d=%c ", prd->fHinges3d ? 'y' : 'n');
	g_string_append_printf(gst, "boardshadows=%c ", prd->showShadows ? 'y' : 'n');
	g_string_append_printf(gst, "shadowdarkness=%d ", prd->shadowDarkness);
	g_string_append_printf(gst, "animateroll=%c ", prd->animateRoll ? 'y' : 'n');
	g_string_append_printf(gst, "animateflag=%c ", prd->animateFlag ? 'y' : 'n');
	g_string_append_printf(gst, "closeboard=%c ", prd->closeBoardOnExit ? 'y' : 'n');
	g_string_append_printf(gst, "quickdraw=%c ", prd->quickDraw ? 'y' : 'n');
	g_string_append_printf(gst, "curveaccuracy=%d ", prd->curveAccuracy);
	g_string_append_printf(gst, "lighttype=%c ", prd->lightType == LT_POSITIONAL ? 'p' : 'd');

	g_ascii_formatd(buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", prd->lightPos[0]);
	g_string_append_printf(gst, "lightposx=%s ", buf);

	g_ascii_formatd(buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", prd->lightPos[1]);
	g_string_append_printf(gst, "lightposy=%s ", buf);
	g_ascii_formatd(buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", prd->lightPos[2]);
	g_string_append_printf(gst, "lightposz=%s ", buf);
	g_string_append_printf(gst, "lightambient=%d ", prd->lightLevels[0]);
	g_string_append_printf(gst, "lightdiffuse=%d ", prd->lightLevels[1]);
	g_string_append_printf(gst, "lightspecular=%d ", prd->lightLevels[2]);
	g_string_append_printf(gst, "boardangle=%d ", prd->boardAngle);
	g_string_append_printf(gst, "skewfactor=%d ", prd->skewFactor);
	g_string_append_printf(gst, "planview=%c ", prd->planView ? 'y' : 'n');

	g_ascii_formatd(buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", prd->diceSize);
	g_string_append_printf(gst, "dicesize=%s ", buf);

	g_string_append_printf(gst, "roundededges=%c ", prd->roundedEdges ? 'y' : 'n');
	g_string_append_printf(gst, "bgintrays=%c ", prd->bgInTrays ? 'y' : 'n');
	g_string_append_printf(gst, "roundedpoints=%c ", prd->roundedPoints ? 'y' : 'n');
	g_string_append_printf(gst, "piecetype=%d ", prd->pieceType);
	g_string_append_printf(gst, "piecetexturetype=%d ", prd->pieceTextureType);
	g_string_append_printf(gst, "chequers3d0=%s ", WriteMaterial(&prd->ChequerMat[0]));
	g_string_append_printf(gst, "chequers3d1=%s ", WriteMaterial(&prd->ChequerMat[1]));
	g_string_append_printf(gst, "dice3d0=%s ", WriteMaterialDice(prd, 0));
	g_string_append_printf(gst, "dice3d1=%s ", WriteMaterialDice(prd, 1));
	g_string_append_printf(gst, "dot3d0=%s ", WriteMaterial(&prd->DiceDotMat[0]));
	g_string_append_printf(gst, "dot3d1=%s ", WriteMaterial(&prd->DiceDotMat[1]));
	g_string_append_printf(gst, "cube3d=%s ", WriteMaterial(&prd->CubeMat));
	g_string_append_printf(gst, "cubetext3d=%s ", WriteMaterial(&prd->CubeNumberMat));
	g_string_append_printf(gst, "base3d=%s ", WriteMaterial(&prd->BaseMat));
	g_string_append_printf(gst, "points3d0=%s ", WriteMaterial(&prd->PointMat[0]));
	g_string_append_printf(gst, "points3d1=%s ", WriteMaterial(&prd->PointMat[1]));
	g_string_append_printf(gst, "border3d=%s ", WriteMaterial(&prd->BoxMat));
	g_string_append_printf(gst, "hinge3d=%s ", WriteMaterial(&prd->HingeMat));
	g_string_append_printf(gst, "numbers3d=%s ", WriteMaterial(&prd->PointNumberMat));
	g_string_append_printf(gst, "background3d=%s ", WriteMaterial(&prd->BackGroundMat));
#endif
	g_string_append_printf(gst, "labels=%c ", prd->fLabels ? 'y' : 'n');
	g_string_append_printf(gst, "dynamiclabels=%c ", prd->fDynamicLabels ? 'y' : 'n');
	g_string_append_printf(gst, "wood=%s ", aszWoodName[prd->wt]);
	g_string_append_printf(gst, "hinges=%c ", prd->fHinges ? 'y' : 'n');

	g_ascii_formatd(buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", rAzimuth);
	g_ascii_formatd(buf1, G_ASCII_DTOSTR_BUF_SIZE, "%f", rElevation);
	g_string_append_printf(gst, "light=%s;%s ", buf, buf1);

	g_ascii_formatd(buf, G_ASCII_DTOSTR_BUF_SIZE, "%0.1f", (1.0 - prd->rRound));
	g_string_append_printf(gst, "shape=%s  ", buf);

	g_ascii_formatd(buf, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->aarColour[0][3]);
	g_ascii_formatd(buf1, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->arRefraction[0]);
	g_ascii_formatd(buf2, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->arCoefficient[0]);
	g_ascii_formatd(buf3, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->arExponent[0]);
	g_string_append_printf(gst, "chequers0=#%02X%02X%02X;%s;%s;%s;%s ", (int) (prd->aarColour[0][0] * 0xFF),
			       (int) (prd->aarColour[0][1] * 0xFF), (int) (prd->aarColour[0][2] * 0xFF), buf, buf1,
			       buf2, buf3);

	g_ascii_formatd(buf, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->aarColour[1][3]);
	g_ascii_formatd(buf1, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->arRefraction[1]);
	g_ascii_formatd(buf2, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->arCoefficient[1]);
	g_ascii_formatd(buf3, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->arExponent[1]);
	g_string_append_printf(gst, "chequers1=#%02X%02X%02X;%s;%s;%s;%s ", (int) (prd->aarColour[1][0] * 0xFF),
			       (int) (prd->aarColour[1][1] * 0xFF), (int) (prd->aarColour[1][2] * 0xFF), buf, buf1,
			       buf2, buf3);

	g_ascii_formatd(buf, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->arDiceCoefficient[0]);
	g_ascii_formatd(buf1, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->arDiceExponent[0]);
	g_string_append_printf(gst, "dice0=#%02X%02X%02X;%s;%s;%c ", (int) (prd->aarDiceColour[0][0] * 0xFF),
			       (int) (prd->aarDiceColour[0][1] * 0xFF), (int) (prd->aarDiceColour[0][2] * 0xFF), buf,
			       buf1, prd->afDieColour[0] ? 'y' : 'n');

	g_ascii_formatd(buf, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->arDiceCoefficient[1]);
	g_ascii_formatd(buf1, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->arDiceExponent[1]);
	g_string_append_printf(gst, "dice1=#%02X%02X%02X;%s;%s;%c ", (int) (prd->aarDiceColour[1][0] * 0xFF),
			       (int) (prd->aarDiceColour[1][1] * 0xFF), (int) (prd->aarDiceColour[1][2] * 0xFF), buf,
			       buf1, prd->afDieColour[1] ? 'y' : 'n');

	g_string_append_printf(gst, "dot0=#%02X%02X%02X ", (int) (prd->aarDiceDotColour[0][0] * 0xFF),
			       (int) (prd->aarDiceDotColour[0][1] * 0xFF), (int) (prd->aarDiceDotColour[0][2] * 0xFF));
	g_string_append_printf(gst, "dot1=#%02X%02X%02X ", (int) (prd->aarDiceDotColour[1][0] * 0xFF),
			       (int) (prd->aarDiceDotColour[1][1] * 0xFF), (int) (prd->aarDiceDotColour[1][2] * 0xFF));
	g_string_append_printf(gst, "cube=#%02X%02X%02X ", (int) (prd->arCubeColour[0] * 0xFF),
			       (int) (prd->arCubeColour[1] * 0xFF), (int) (prd->arCubeColour[2] * 0xFF));

	g_ascii_formatd(buf, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->aSpeckle[2] / 128.0);
	g_string_append_printf(gst, "points0=#%02X%02X%02X;%s ", prd->aanBoardColour[2][0], prd->aanBoardColour[2][1],
			       prd->aanBoardColour[2][2], buf);

	g_ascii_formatd(buf, G_ASCII_DTOSTR_BUF_SIZE, "%.2f", prd->aSpeckle[3] / 128.0);
	g_string_append_printf(gst, "points1=#%02X%02X%02X;%s", prd->aanBoardColour[3][0], prd->aanBoardColour[3][1],
			       prd->aanBoardColour[3][2], buf);

	return gst;
}
