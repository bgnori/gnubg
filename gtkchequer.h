/*
 * gtkchequer.h
 *
 * by Joern Thyssen <jthyssen@dk.ibm.com>, 2002
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
 * $Id: gtkchequer.h,v 1.8 2005/08/29 14:56:06 Superfly_Jon Exp $
 */

#ifndef _GTKCHEQUER_H_
#define _GTKCHEQUER_H_

typedef struct _hintdata {
  GtkWidget *pwMoves;     /* the movelist */
  GtkWidget *pwRollout, *pwRolloutSettings; /* rollout buttons */
  GtkWidget *pwEval, *pwEvalSettings;       /* evaluation buttons */
  GtkWidget *pwMove; /* move button */
  GtkWidget *pwCopy; /* copy button */
  GtkWidget *pwEvalPly; /* predefined eval buttons */
  GtkWidget *pwShow; /* button for showing moves */
  GtkWidget *pwTempMap; /* button for showing temperature map */
  movelist *pml;
  int fButtonsValid;
  int fDestroyOnMove;
  int *piHighlight;
  int fDetails;
} hintdata;

extern GtkWidget *
CreateMoveList( movelist *pml, int *piHighlight, 
                const int fButtonsValid, const int fDestroyOnMove,
                const int fDetails );

extern int 
CheckHintButtons( hintdata *phd );

extern void MoveListRefreshSize();
extern GtkWidget* pwDetails;
extern int showMoveListDetail;
#endif
