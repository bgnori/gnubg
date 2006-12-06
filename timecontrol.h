/*
 * timecontrol.h
 *
 * by Stein Kulseth <steink@opera.com>, 2003
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
 * $Id: timecontrol.h,v 1.10 2006/12/06 23:12:52 c_anthon Exp $
 */

#ifndef _TIMECONTROL_H_
#define  _TIMECONTROL_H_

#include <stdio.h>
#include <time.h>

#if USE_GTK
#include <gtk/gtk.h>
#endif

#include "backgammon.h"

#if USE_TIMECONTROL

extern timecontrol tc;

/* Initialise GameClock at start of match
 * @param pgc pointer to gameclock to initialise
 * @param ptc pointer to timecontrol to use
 * @param nPoints length of match
 */
extern void InitGameClock(gameclock *pgc, timecontrol *ptc, int nPoints);

/* Start the game clock for fPlayer 
 * Proper sequence for hitting the clock is :
 * - call CheckGameClock to update the game clock and check for timeout
 * -- apply any penalties that apply
 * - resolve any action done (move/double/drop. etc ... ) including
 *   updating ms.fTurn to its proper value.
 * - call HitGameClock
 * @param pms pointer to matchstate 
 */
extern void HitGameClock(matchstate *pms);


/* Updates the Game clock and checks whether the time has run out.
 * for the player whose clock is running
 * @param pms pointer to matchstate
 * @param tvp timestamp for hit
 * @return how many penalty points to apply
 */
extern int CheckGameClock(matchstate *pms, struct timeval *tvp);

/* Make a formatted string for the given player.
 * @param ptl pointer to  time to format
 * @param buf pointer to result buffer - if 0 a static buf is used
 *	but this is rewritten on subsequent calls to FormatClock
 * @return Remaining clock time on the format h:mm:ss 
 */
extern char *FormatClock(struct timeval * ptl , char *buf);

/* Force a (real time) update to the clock which again will 
 * issue a 
 * MOVE_TIME record if time has run out
 */ 
#if USE_GTK
extern gboolean UpdateClockNotify(gpointer *p); 
#else
extern int UpdateClockNotify(void *p);
#endif


/* Save time settings
 * @param open settings file
 */

extern void SetDefaultTC ();
extern void SaveTimeControlSettings( FILE *pf );
extern void CommandShowTCTutorial ();

#endif /* USE_TIMECONTROL */
#endif /* _TIMECONTROL_H_ */
